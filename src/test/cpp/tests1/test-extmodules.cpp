/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include "TExtModule.hpp"

#include "extmodule/CBregexpDll2.h"
#include "extmodule/CMigemo.h"
#include "extmodule/CUchardet.h"
//#include "extmodule/CUxTheme.h"		//TSingletonなのでテストできない
#include "macro/CPPA.h"
//#include "plugin/CDllPlugin.h"		//継承不可なのでテストできない

#include "env/CShareData.h"

#include "tests1_rc.h"

void extract_zip_resource(WORD id, const std::optional<std::filesystem::path>& optOutDir);

/*!
	外部DLLの読み込みテスト
 */
template <typename T>
class LoadTest : public ::testing::Test {
};

//! パラメータテストであるとマークする
TYPED_TEST_CASE_P(LoadTest);

/*!
	読み込み失敗のテスト

	DLLが見つからなくて失敗するケース
 */
TYPED_TEST_P(LoadTest, FailedToLoadLibrary)
{
	// テスト対象のCDllImpl派生クラスをテストするための型を定義する
	using ExtModule = TUnresolvedExtModule<TypeParam>;

	// テストクラスをインスタンス化する
	ExtModule extModule;

	// DLLが見つからなくてIsAvailableはfalseになる
	EXPECT_FALSE(extModule.IsAvailable());
}

/*!
	読み込み失敗のテスト

	エクスポート関数が取得できなくて失敗するケース
 */
TYPED_TEST_P(LoadTest, FailedToGetProcAddress)
{
	// テスト対象のCDllImpl派生クラスをテストするための型を定義する
	using ExtModule = TUnsufficientExtModule<TypeParam>;

	// テストクラスをインスタンス化する
	ExtModule extModule;

	// エクスポート関数の一部が見つからなくてIsAvailableはfalseになる
	EXPECT_FALSE(extModule.IsAvailable());
}

// test suiteを登録する
REGISTER_TYPED_TEST_SUITE_P(
	LoadTest,
	FailedToLoadLibrary, FailedToGetProcAddress);

/*!
	テスト対象（外部DLLを読み込むクラス）

	CDllImp派生クラスである必要がある。
 */
using ExtModuleImplementations = ::testing::Types<
	CBregexpDll2,
	CMigemo,
	CUchardet,
	CPPA>;

//! パラメータテストをインスタンス化する
INSTANTIATE_TYPED_TEST_SUITE_P(
	ExtModule,
	LoadTest,
	ExtModuleImplementations);

struct CMigemoTest : public ::testing::Test {
	using CShareDataHolder = std::unique_ptr<CShareData>;
	using CMigemoHolder = std::unique_ptr<CMigemo>;

	static inline CShareDataHolder pcShareData = nullptr;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		pcShareData = std::make_unique<CShareData>();
		EXPECT_THAT( pcShareData->InitShareData(), IsTrue() );
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		pcShareData = nullptr;
	}

	CMigemoHolder pcMigemo = nullptr;

	/*!
	 * 辞書ファイルのパス
	 */
	std::filesystem::path dictPath;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// 辞書ファイルのパスを取得
		dictPath = GetIniFileName().replace_filename(L"dict");

		// 辞書ファイルを削除する
		if (fexist(dictPath)) {
			std::filesystem::remove_all(dictPath);
		}

		// テストクラスをインスタンス化する
		pcMigemo = std::make_unique<CMigemo>();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// テストクラスのインスタンスを破棄する
		pcMigemo = nullptr;

		// 辞書ファイルを削除する
		if (fexist(dictPath)) {
			std::error_code ec;
			std::filesystem::remove_all(dictPath, ec);
		}
	}
};

TEST_F(CMigemoTest, test001)
{
	// 共有メモリを破棄して初期状態にする
	pcShareData = nullptr;

	// 初期状態では利用不可
	EXPECT_THAT(pcMigemo->IsAvailable(), IsFalse());

	// 初期状態では利用不可
	EXPECT_THAT(pcMigemo->migemo_is_enable(), IsFalse());

	// ロード前は辞書ロードも利用不可、共有メモリ初期化前は例外発生
	EXPECT_ANY_THROW(pcMigemo->migemo_load_all());

	// 名前を指定しないロードは、共有メモリ初期化前にはできない
	EXPECT_ANY_THROW(pcMigemo->InitDll());

	// 名前を指定してロードする
	pcMigemo->InitDll(L"migemo.dll");

	// ロードされたら利用可能になる
	EXPECT_THAT(pcMigemo->IsAvailable(), IsTrue());

	// 共有メモリを初期化する
	pcShareData = std::make_unique<CShareData>();
	EXPECT_THAT(pcShareData->InitShareData(), IsTrue());

	// C/Migemo設定に値を入れる
	::wcsncpy_s(GetDllShareData().m_Common.m_sHelper.m_szMigemoDll, L"", _TRUNCATE);
	::wcsncpy_s(GetDllShareData().m_Common.m_sHelper.m_szMigemoDict, L"", _TRUNCATE);

	// ロードされたら辞書ロードも利用可能になるが辞書がないのでロードに失敗する
	EXPECT_THAT(pcMigemo->migemo_load_all(), IsTrue()); // FIXME: 辞書が読めなくても1を返してしまう不具合

	// 辞書がないので結局利用不可
	EXPECT_THAT(pcMigemo->migemo_is_enable(), IsFalse());
}

TEST_F(CMigemoTest, test002)
{
	// C/Migemo設定に値を入れる
	::wcsncpy_s(GetDllShareData().m_Common.m_sHelper.m_szMigemoDll, L"migemo.dll", _TRUNCATE);
	::wcsncpy_s(GetDllShareData().m_Common.m_sHelper.m_szMigemoDict, L"dict", _TRUNCATE);

	// 名前を指定せずにロードする
	pcMigemo->InitDll();

	// 辞書ファイルを展開する
	extract_zip_resource(IDR_ZIPRES2, dictPath);

	// ロードされたら辞書ロードも利用可能になる
	EXPECT_THAT(pcMigemo->migemo_load_all(), IsTrue());

	// 辞書が見付かるので利用可能になる
	EXPECT_THAT(pcMigemo->migemo_is_enable(), IsTrue());

	// 与えた文字列に対する正規表現が返る
	EXPECT_THAT(pcMigemo->migemo_query_w(L"さくらえでぃt"), StrEq(L"(ｻｸﾗｴﾃﾞｨ[ﾄﾃﾂﾁﾀ]|サクラエディ[トテツチタ]|さくらえでぃ[とてつちたｔt])"));
}

TEST_F(CMigemoTest, test003)
{
	// C/Migemo設定に値を入れる
	::wcsncpy_s(GetDllShareData().m_Common.m_sHelper.m_szMigemoDll, GetIniFileName().replace_filename( L"migemo.dll" ).c_str(), _TRUNCATE);
	::wcsncpy_s(GetDllShareData().m_Common.m_sHelper.m_szMigemoDict, dictPath.c_str(), _TRUNCATE);

	// 名前を指定せずにロードする
	pcMigemo->InitDll();

	// 辞書ファイルを展開する
	extract_zip_resource(IDR_ZIPRES3, dictPath);

	// ロードされたら辞書ロードも利用可能になる
	EXPECT_THAT(pcMigemo->migemo_load_all(), IsTrue());

	// 辞書が見付かるので利用可能になる
	EXPECT_THAT(pcMigemo->migemo_is_enable(), IsTrue());

	// 与えた文字列をSJISに変換できない場合、与えた文字列がそのまま返る
	EXPECT_THAT(pcMigemo->migemo_query_w(L"\U0001F6B9"), StrEq(L"\U0001F6B9"));
}
