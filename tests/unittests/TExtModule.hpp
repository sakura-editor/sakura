/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

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
#include <gtest/gtest.h>

#include "extmodule/CDllHandler.h"

#include <type_traits>

/*!
	外部DLL読み込みをテストするためのテンプレートクラス

	CDllImp派生クラスを指定して使う。
	テストではロード処理後の挙動をチェックするので、コンストラクタでinitしてしまう。
 */
template<class T, std::enable_if_t<std::is_base_of_v<CDllImp, T>, std::nullptr_t> = nullptr>
class TExtModule : public T {
private:
	std::wstring_view dllName = L"";

protected:
	//! DLLパスを返す
	LPCWSTR GetDllNameImp(int index) override {
		return dllName.empty() ? T::GetDllNameImp(index) : dllName.data();
	}

public:
	//! コンストラクタ
	explicit TExtModule(std::wstring_view path = L"")
		: dllName(path) {
		// この関数の戻り値型はC++では推奨されない。
		// あちこちで警告が出るとうっとおしいので呼出箇所をまとめておく
		// 将来的には、適切な戻り値型に変更したい。
		this->InitDll(nullptr);
	}
};

/*!
	外部DLLの読み込み失敗をテストするためのテンプレートクラス

	CDllImp派生クラスを指定して使う。
	DLL読み込み失敗をテストするために「あり得ないパス」を指定する。
 */
template<class T>
class TUnresolvedExtModule : public TExtModule<T> {
private:
	// あり得ないパス
	static constexpr auto& BadDllName = LR"(>\(^o^)\<)";

	// 基底クラスの型
	using Base = TExtModule<T>;

public:
	TUnresolvedExtModule()
		: Base(BadDllName) {
	}
};

/*!
	外部DLLの読み込み失敗をテストするためのテンプレートクラス

	CDllImp派生クラスを指定して使う。
	エクスポート関数のアドレス取得失敗をテストするためにMSHTMLを指定する。
	MSHTMLは、Windowsには必ず存在していてサクラエディタでは使わないもの。
	将来的にMSHTMLを使いたくなったら他のDLLを選定して定義を修正すること。
 */
template<class T>
class TUnsufficientExtModule : public TExtModule<T> {
private:
	// サクラエディタでは利用しないWindowsのDLL名
	static constexpr auto& UnusedWindowsDllName = L"MSHTML.DLL";

	// 基底クラスの型
	using Base = TExtModule<T>;

public:
	TUnsufficientExtModule()
		: Base(UnusedWindowsDllName) {
	}
};
