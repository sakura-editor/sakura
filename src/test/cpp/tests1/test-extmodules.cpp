/*! @file */
/*
	Copyright (C) 2022-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include "TExtModule.hpp"

#include "extmodule/CBregexp.h"
#include "extmodule/CMigemo.h"
#include "extmodule/CUchardet.h"
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
	CBregOnig,
	CMigemo,
	CUchardet,
	CPPA>;

//! パラメータテストをインスタンス化する
INSTANTIATE_TYPED_TEST_SUITE_P(
	ExtModule,
	LoadTest,
	ExtModuleImplementations);

namespace extmodule {

struct CBregexpTest : public ::testing::Test {
	using CShareDataHolder = std::unique_ptr<CShareData>;
	using CBregexpHolder = std::unique_ptr<CBregexp>;

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

	CBregexpHolder pcBregexp = nullptr;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		// テストクラスをインスタンス化する
		pcBregexp = std::make_unique<CBregexp>();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		// テストクラスのインスタンスを破棄する
		pcBregexp = nullptr;
	}
};

TEST_F(CBregexpTest, test001)
{
	// 初期状態では利用不可
	EXPECT_THAT(pcBregexp->IsAvailable(), IsFalse());

	// ロード前はバージョン情報も取得できない
	EXPECT_THAT(pcBregexp->GetVersionW(), StrEq(L""));

	// ロード前は正規表現コンパイルできない
	EXPECT_THAT(pcBregexp->Compile(L"[0-9]+", L"$1d"), IsFalse());

	// C/Migemo設定に値を入れる
	::wcsncpy_s(GetDllShareData().m_Common.m_sSearch.m_szRegexpLib, L"", _TRUNCATE);

	// 名前を指定せずにロードする
	pcBregexp->InitDll();

	// ロードされたら利用可能になる
	EXPECT_THAT(pcBregexp->IsAvailable(), IsTrue());

	// ロードされたらバージョン情報が取れる
	EXPECT_THAT(pcBregexp->GetVersionW(), StrNe(L""));

	EXPECT_THAT(pcBregexp->GetIndex(), 0);
	EXPECT_THAT(pcBregexp->GetLastIndex(), 0);
	EXPECT_THAT(pcBregexp->GetMatchLen(), 0);
	EXPECT_THAT(pcBregexp->GetStringLen(), 0);
	EXPECT_THAT(pcBregexp->GetString(), StrEq(L""));
	EXPECT_THAT(pcBregexp->GetLastMessage(), StrEq(L""));

	// 正規表現コンパイルが成功する
	EXPECT_THAT(pcBregexp->Compile(L".+"), IsTrue());
	EXPECT_THAT(pcBregexp->Compile(L"^$"), IsTrue());

	// オプションビットをすべて立てても失敗しない
	EXPECT_THAT(pcBregexp->Compile(L"[0-9]+", int(-1)), IsTrue());

	// 構文エラーでコンパイルエラーを発生させる
	EXPECT_THAT(pcBregexp->Compile(L"[0-9"), IsFalse());
	EXPECT_THAT(pcBregexp->GetLastMessage(), StrNe(L""));

	// 正規表現コンパイルを成功させる
	EXPECT_THAT(pcBregexp->Compile(L"[0-9]+"), IsTrue());

	auto pattern = pcBregexp->GetPattern();
	EXPECT_THAT(pattern, NotNull());
	EXPECT_THAT(pcBregexp->GetPattern(), IsNull());

	std::wstring msg(79, L'\0');
	pattern = std::make_unique<CBregexp::CPattern>(*pcBregexp, nullptr, msg);

	EXPECT_THAT(pattern->Match(L"test"), IsFalse());
	EXPECT_THAT(pattern->Replace(L"test"), IsFalse());
}

TEST_F(CBregexpTest, test002)
{
	// ロード前は検索できない
	EXPECT_THAT(pcBregexp->Match(L"test123あいう"), IsFalse());

	// C/Migemo設定に値を入れる
	::wcsncpy_s(GetDllShareData().m_Common.m_sSearch.m_szRegexpLib, L"bregonig.dll", _TRUNCATE);

	// 名前を指定せずにロードする
	pcBregexp->InitDll();

	// ロードされたら利用可能になる
	EXPECT_THAT(pcBregexp->IsAvailable(), IsTrue());

	// 正規表現コンパイルが成功する
	EXPECT_THAT(pcBregexp->Compile(L"[0-9]+"), IsTrue());

	// マッチ実行前は0が返る
	EXPECT_THAT(pcBregexp->GetMatchLen(), 0);

	// 正規表現マッチが成功する
	EXPECT_THAT(pcBregexp->Match(L"test123あいう"), IsTrue());
	EXPECT_THAT(pcBregexp->GetLastMessage(), StrEq(L""));

	EXPECT_THAT(pcBregexp->GetIndex(), 4);
	EXPECT_THAT(pcBregexp->GetLastIndex(), 7);
	EXPECT_THAT(pcBregexp->GetMatchLen(), 3);

	// マッチしないパターンの確認
	EXPECT_THAT(pcBregexp->Match(L"a", 1), IsFalse());
}

TEST_F(CBregexpTest, test003)
{
	// ロード前は置換できない
	EXPECT_THAT(pcBregexp->Replace(L"test123あいう"), IsFalse());

	// C/Migemo設定に値を入れる
	::wcsncpy_s(GetDllShareData().m_Common.m_sSearch.m_szRegexpLib, GetIniFileName().replace_filename(L"bregonig.dll").c_str(), _TRUNCATE);

	// 名前を指定せずにロードする
	pcBregexp->InitDll();

	// ロードされたら利用可能になる
	EXPECT_THAT(pcBregexp->IsAvailable(), IsTrue());

	// 正規表現コンパイルが成功する
	EXPECT_THAT(pcBregexp->Compile(L"([0-9]+)", L"{$1d}"), IsTrue());

	// 正規表現置換が成功する
	EXPECT_THAT(pcBregexp->Replace(L"test123あいう"), IsTrue());
	EXPECT_THAT(pcBregexp->GetLastMessage(), StrEq(L""));

	// 置換結果を確認する
	constexpr auto& expected = L"test{123d}あいう";
	EXPECT_THAT(pcBregexp->GetReplacedString(), StrEq(expected));
	EXPECT_THAT(pcBregexp->GetString(), StrEq(expected));
	EXPECT_THAT(pcBregexp->GetStringLen(), std::size(expected) - 1);

	// マッチしないパターンの確認
	EXPECT_THAT(pcBregexp->Replace(L"a", 1), IsFalse());
	EXPECT_THAT(pcBregexp->GetMatchLen(), 0);
}

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

	// ロード前は辞書ロードも利用不可
	EXPECT_THAT(pcMigemo->migemo_load_all(), IsFalse());

	// 名前を指定しないロードは、共有メモリ初期化前にはできない
	EXPECT_ANY_THROW(pcMigemo->InitDll());

	// 名前を指定してロードする
	pcMigemo->InitDll(L"migemo.dll");

	// ロードされたら利用可能になる
	EXPECT_THAT(pcMigemo->IsAvailable(), IsTrue());

	// テストクラスのインスタンスを破棄する
	pcMigemo = nullptr;

	// テストクラスをインスタンス化する
	pcMigemo = std::make_unique<CMigemo>();

	// 共有メモリを初期化する
	pcShareData = std::make_unique<CShareData>();
	EXPECT_THAT(pcShareData->InitShareData(), IsTrue());

	// C/Migemo設定に値を入れる
	::wcsncpy_s(GetDllShareData().m_Common.m_sHelper.m_szMigemoDll, L"", _TRUNCATE);
	::wcsncpy_s(GetDllShareData().m_Common.m_sHelper.m_szMigemoDict, L"", _TRUNCATE);

	// 名前を指定せずにロードする
	pcMigemo->InitDll();

	// ロードされたら辞書ロードも利用可能になるが辞書がないのでロードに失敗する
	EXPECT_THAT(pcMigemo->migemo_load_all(), IsFalse());

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

} // namespace extmodule
