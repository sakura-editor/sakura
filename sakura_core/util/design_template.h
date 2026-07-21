/*! @file */
/*
2007.10.23 kobake

デザインパターン的なモノを置けると良いなぁ。
ちなみに TSingleInstance はシングルトンパターンとは似て非なるモノですが。
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_DESIGN_TEMPLATE_BBC57590_CED0_40D0_B719_F5A4522B8A56_H_
#define SAKURA_DESIGN_TEMPLATE_BBC57590_CED0_40D0_B719_F5A4522B8A56_H_
#pragma once

#include <algorithm>
#include <concepts>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <type_traits>
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
 * @brief サクラエディタの変則Singletonパターン。
 *
 * TSingletonを元に作成。
 * 生成したインスタンスをリセットできるようにしてある。
 */
template <class T>
class TSakuraSingleton {
private:
	using Me = TSakuraSingleton<T>;

	//! 生成済みインスタンス
	static inline std::unique_ptr<T> gm_Instance = nullptr;

	//! インスタンス生成用の排他制御用ミューテックス
	static inline std::mutex gm_Mutex;

public:
	/*!
	 * @brief インスタンスポインタを取得する
	 *
	 * @returns インスタンスポインタ
	 * @note インスタンスが未生成の場合は生成する
	 */
	static T* getInstance()
	{
		static_assert(
			std::default_initializable<T> && !std::is_array_v<T>,
			"T must be publicly default-initializable and must not be an array"
		);

		std::unique_lock lock{ gm_Mutex };

		if (!gm_Instance) {
			gm_Instance = std::make_unique<T>();
		}

		return gm_Instance.get();
	}

	/*!
	 * @brief インスタンスを破棄する
	 *
	 * @note 取得したポインタを破棄後に使用しないこと
	 */
	static void resetInstance()
	{
		std::unique_lock lock{ gm_Mutex };

		gm_Instance.reset();
	}

protected:
	TSakuraSingleton() = default;

public:
	TSakuraSingleton(const Me&) = delete;
	Me& operator = (const Me&) = delete;

	TSakuraSingleton(Me&&) = delete;
	Me& operator = (Me&&) = delete;
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
