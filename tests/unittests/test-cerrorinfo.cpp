/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>

#include <wrl.h>

#include "basis/CErrorInfo.h"

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, AddRef_Release)
{
	// IErrorInfoを生成する
	IErrorInfo* pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(L"test")));

	// 初期値は0なので、AddRefすると1が返る
	ASSERT_EQ(1, pErrorInfo->AddRef());

	// 内部カウンタは1なので、Releaseすると0になる
	ASSERT_EQ(0, pErrorInfo->Release());
}

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, QueryInterface_BadPointer)
{
	// IErrorInfoを生成する
	IErrorInfo* pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(L"test")));

	// 初期値は0なので、AddRefすると1が返る
	ASSERT_EQ(1, pErrorInfo->AddRef());

	// QueryInterfaceのOutポインタにnullptrを指定するとポインタエラーになる
	ASSERT_EQ(E_POINTER, pErrorInfo->QueryInterface(GUID_NULL, nullptr));

	// 内部カウンタは1なので、Releaseすると0になる
	ASSERT_EQ(0, pErrorInfo->Release());
}

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, QueryInterface_IUnknown)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	IErrorInfo* pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(L"test")));

	// 初期値は0なので、AddRefすると1が返る
	ASSERT_EQ(1, pErrorInfo->AddRef());

	// IUnknownを取得する
	ComPtr<IUnknown> pUnknown;
	ASSERT_TRUE(SUCCEEDED(pErrorInfo->QueryInterface(pUnknown.GetAddressOf())));

	//pUnknownを解放する
	pUnknown = nullptr;

	// 内部カウンタは1に戻っているはずなので、Releaseすると0になる
	ASSERT_EQ(0, pErrorInfo->Release());
}

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, QueryInterface_Implemented)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	IErrorInfo* pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(L"test")));

	// 初期値は0なので、AddRefすると1が返る
	ASSERT_EQ(1, pErrorInfo->AddRef());

	// IErrorInfoを取得する
	ComPtr<IErrorInfo> pOther;
	ASSERT_TRUE(SUCCEEDED(pErrorInfo->QueryInterface(pOther.GetAddressOf())));

	//pOtherを解放する
	pOther = nullptr;

	// 内部カウンタは1に戻っているはずなので、Releaseすると0になる
	ASSERT_EQ(0, pErrorInfo->Release());
}

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, QueryInterface_NotImplemented)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	IErrorInfo* pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(L"test")));

	// 初期値は0なので、AddRefすると1が返る
	ASSERT_EQ(1, pErrorInfo->AddRef());

	// ICreateErrorInfo(実装されていないインターフェース)の取得を試みる
	ComPtr<ICreateErrorInfo> pCreateErrorInfo;
	ASSERT_EQ(E_NOINTERFACE, pErrorInfo->QueryInterface(pCreateErrorInfo.GetAddressOf()));

	// 内部カウンタは1なので、Releaseすると0になる
	ASSERT_EQ(0, pErrorInfo->Release());
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetGUID_BadPointer)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// GUIDを取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetGUID(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetGUID)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// guidを取得する
	GUID guid = GUID_NULL;
	ASSERT_TRUE(SUCCEEDED(pErrorInfo->GetGUID(&guid)));
	ASSERT_EQ(GUID_NULL, guid);
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetSource_BadPointer)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// ソース情報を取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetSource(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetSource)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// ソース情報を取得する
	_bstr_t bstrSource;
	ASSERT_TRUE(SUCCEEDED(pErrorInfo->GetSource(bstrSource.GetAddress())));
	// 期待値を作るのが面倒なので比較は省略
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetDescription_BadPointer)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// 説明を取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetDescription(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetDescription)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// 説明を取得する
	_bstr_t bstrDescription;
	ASSERT_TRUE(SUCCEEDED(pErrorInfo->GetDescription(bstrDescription.GetAddress())));
	ASSERT_STREQ(msg, (const wchar_t*)bstrDescription);
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetHelpFile_BadPointer)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// ヘルプファイルのパスを取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetHelpFile(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetHelpFile)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// ヘルプファイルのパスを取得する
	_bstr_t bstrHelpFile;
	ASSERT_TRUE(SUCCEEDED(pErrorInfo->GetHelpFile(bstrHelpFile.GetAddress())));
	ASSERT_EQ(nullptr, (const wchar_t*)bstrHelpFile);
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetHelpContext_BadPointer)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// ヘルプコンテキストを取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetHelpContext(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetHelpContext)
{
	using namespace Microsoft::WRL;

	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	ComPtr<IErrorInfo> pErrorInfo(reinterpret_cast<IErrorInfo*>(MakeMsgError(msg)));

	// ヘルプコンテキストを取得する
	DWORD dwHelpContext;
	ASSERT_TRUE(SUCCEEDED(pErrorInfo->GetHelpContext(&dwHelpContext)));
	ASSERT_EQ(0, dwHelpContext);
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, StandardUsageTest)
{
	// テストに使う日本語メッセージ
	constexpr const wchar_t message[] = L"エラーが発生しました！";

	try {
		// エラー情報を生成して例外を投げる
		::_com_raise_error(E_FAIL, MakeMsgError(message));

		// 例外を投げたあとのコードは実行されない
		FAIL();
	}
	catch (const _com_error& ce) {
		// 投げられた例外メッセージを取得できること
		ASSERT_STREQ(message, (const wchar_t*)ce.Description());
	}
}
