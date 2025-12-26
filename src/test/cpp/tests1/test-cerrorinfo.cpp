/*! @file */
/*
	Copyright (C) 2021-2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "basis/CErrorInfo.h"

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, AddRef_Release)
{
	// IErrorInfoを生成する
	auto pErrorInfo = MakeMsgError(L"test");

	// 初期値は1なので、AddRefすると2が返る
	EXPECT_THAT(pErrorInfo->AddRef(), 2);

	// 内部カウンタは2なので、Releaseすると1になる
	ASSERT_THAT(pErrorInfo->Release(), 1);
}

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, QueryInterface_BadPointer)
{
	// IErrorInfoを生成する
	auto pErrorInfo = MakeMsgError(L"test");

	// QueryInterfaceのOutポインタにnullptrを指定するとポインタエラーになる
	EXPECT_THAT(pErrorInfo->QueryInterface(GUID_NULL, nullptr), E_POINTER);
}

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, QueryInterface_IUnknown)
{
	// IErrorInfoを生成する
	auto pErrorInfo = MakeMsgError(L"test");

	// IUnknownを取得する（内部カウンタは2になる）
	cxx::com_pointer<IUnknown> pUnknown;
	EXPECT_THAT(pErrorInfo->QueryInterface(&pUnknown), S_OK);

	EXPECT_THAT(pUnknown, NotNull());

	//pUnknownを解放する
	pUnknown = nullptr;

	EXPECT_THAT(pUnknown, IsNull());

	// 内部カウンタは1に戻っているので、Releaseしない
}

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, QueryInterface_Implemented)
{
	// IErrorInfoを生成する
	auto pErrorInfo = MakeMsgError(L"test");

	// IErrorInfoを取得する（内部カウンタは2になる）
	cxx::com_pointer<IErrorInfo> pOther;
	EXPECT_THAT(pErrorInfo->QueryInterface(&pOther), S_OK);

	EXPECT_THAT(pOther, NotNull());

	//pOtherを解放する
	pOther = nullptr;

	EXPECT_THAT(pOther, IsNull());

	// 内部カウンタは1に戻っているので、Releaseしない
}

/*!
 * @brief テンプレートクラスの機能確認
 */
TEST(TComImpl, QueryInterface_NotImplemented)
{
	// IErrorInfoを生成する
	auto pErrorInfo = MakeMsgError(L"test");

	// ICreateErrorInfo(実装されていないインターフェース)の取得を試みる
	cxx::com_pointer<ICreateErrorInfo> pCreateErrorInfo;
	EXPECT_THAT(pErrorInfo->QueryInterface(&pCreateErrorInfo), E_NOINTERFACE);

	// 内部カウンタは1なので、Releaseしない
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetGUID_BadPointer)
{
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

	// GUIDを取得する
	EXPECT_THAT(pErrorInfo->GetGUID(nullptr), E_POINTER);
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetGUID)
{
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

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
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

	// ソース情報を取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetSource(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetSource)
{
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

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
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

	// 説明を取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetDescription(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetDescription)
{
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

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
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

	// ヘルプファイルのパスを取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetHelpFile(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetHelpFile)
{
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

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
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

	// ヘルプコンテキストを取得する
	ASSERT_EQ(E_POINTER, pErrorInfo->GetHelpContext(nullptr));
}

/*!
 * @brief COMエラークラスの機能確認
 */
TEST(CErrorInfo, GetHelpContext)
{
	// IErrorInfoを生成する
	constexpr const wchar_t msg[] = L"test";
	auto pErrorInfo = MakeMsgError(msg);

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
		::_com_raise_error(E_FAIL, MakeMsgError(message).Detach());

		// 例外を投げたあとのコードは実行されない
		FAIL();
	}
	catch (const _com_error& ce) {
		// 投げられた例外メッセージを取得できること
		EXPECT_THAT(LPCWSTR(ce.Description()), StrEq(message));
	}
}
