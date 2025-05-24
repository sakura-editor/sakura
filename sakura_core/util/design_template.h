/*! @file */
/*
2007.10.23 kobake

デザインパターン的なモノを置けると良いなぁ。
ちなみに TSingleInstance はシングルトンパターンとは似て非なるモノですが。
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_DESIGN_TEMPLATE_BBC57590_CED0_40D0_B719_F5A4522B8A56_H_
#define SAKURA_DESIGN_TEMPLATE_BBC57590_CED0_40D0_B719_F5A4522B8A56_H_
#pragma once

#include <algorithm>
#include <stdexcept>
#include <vector>

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
	複数インスタンスを生成しようとしたときのエラー
 */
class multi_instance_not_allowed : public std::domain_error {
public:
	multi_instance_not_allowed()
		: std::domain_error("multi instance not allowed.") {}
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
	using Me = TSingleInstance<T>;

	static T* gm_instance;				//!< シングルインスタンスを保持するポインタ

public:
	/*!
		作成済みのインスタンスを取得する

		@returns 作成済みのインスタンス
		@retval nullptr インスタンスが未生成
	 */
	[[nodiscard]] static T* getInstance() noexcept { return gm_instance; }

	TSingleInstance(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	TSingleInstance(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;

protected:
	/*!
		コンストラクタ

		staticメンバにインスタンスを記録する。
	 */
	TSingleInstance()
	{
		if (gm_instance != nullptr) {
			throw multi_instance_not_allowed();
		}
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

/*!
	インスタンスホルダー

	プロセス内で生成されたインスタンスを記録するためのテンプレート。
	インスタンスホルダーは複数インスタンスを生成するクラスに適用する。
	インスタンスホルダーの生成済みのインスタンスはstaticメソッドから取得できる。
	インスタンス自動生成は行わないので、インスタンス生成は手動で行うこと。

	デザインパターンの「シングルトン」とは関係ないので、派生クラスは「状態」を持って良い。
 */
template <class T>
class TInstanceHolder {
private:
	using Me = TInstanceHolder<T>;

	static std::vector<T*> gm_table;	//!< インスタンスを保持する動的配列

public:
	/*!
		作成済みのインスタンス数を取得する

		@returns 作成済みのインスタンス数
	 */
	[[nodiscard]] static size_t GetInstanceCount() noexcept { return gm_table.size(); }

	/*!
		作成済みのインスタンスを取得する

		@param [in]index
		@returns 作成済みのインスタンス
		@retval nullptr インスタンスが未生成
	 */
	[[nodiscard]] static T* GetInstance(size_t index) noexcept
	{
		if (gm_table.size() <= index || gm_table.empty()) {
			return nullptr;
		}
		return gm_table[index];
	}

	/*!
		作成済みのインスタンスを取得する

		@returns 作成済みのインスタンス
		@retval nullptr インスタンスが未生成
	 */
	[[nodiscard]] static T* getInstance() noexcept { return GetInstance(0); }

	TInstanceHolder(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	TInstanceHolder(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;

protected:
	/*!
		コンストラクタ

		staticメンバにインスタンスを記録する。
	 */
	TInstanceHolder()
	{
		gm_table.push_back(static_cast<T*>(this));
	}

	/*!
		デストラクタ

		staticメンバからこのインスタンスのポインタを除去する。
	 */
	virtual ~TInstanceHolder() noexcept
	{
		if (const auto it = std::find(gm_table.cbegin(), gm_table.cend(), this);
			it != gm_table.cend())
		{
			gm_table.erase(it);
		}
	}
};

/*!
	インスタンスを保持する動的配列

	プロセスで生成したインスタンスを記録する機構で、
	TInstanceHolder<T>以外からはアクセスさせない。
 */
template <class T>
std::vector<T*> TInstanceHolder<T>::gm_table;

#endif /* SAKURA_DESIGN_TEMPLATE_BBC57590_CED0_40D0_B719_F5A4522B8A56_H_ */
