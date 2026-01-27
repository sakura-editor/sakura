/*! @file */
/*
	Copyright (C) 2021-2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "io/CZipFile.h"

#include "cxx/lock_resource.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <locale>
#include <string>

#include <miniz-cpp/zip_file.hpp>

#include "util/file.h"

#include "tests1_rc.h"
#include "rt_zipres.h"

using BinarySequence = std::basic_string<std::byte>;
using BinarySequenceView = std::basic_string_view<std::byte>;

/*!
	バイナリデータをファイルに書き込む

	@param [in] bin バイナリデータ
	@param [in] path 書き込み先ファイルパス
 */
bool WriteBinaryToFile(BinarySequenceView bin, std::filesystem::path path)
{
	if (bin.empty()) {
		return false;
	}

	try {
		// 内部的なストリームインスタンスを用意する
		// std::byteでパラメータ化したstd::basic_ofstreamだとMinGWビルドが動作しないので、
		// あえて標準の1バイト実装を使う
		std::ofstream os{ path, std::ios::binary | std::ios::trunc };

		if (!os) {
			return false;
		}

		os.write(std::bit_cast<const char*>(bin.data()), bin.length());
	}
	catch (...) {
		return false;
	}

	return true;
}

/*!
 * 新しいテンポラリファイルパスを生成する
 * （拡張子を指定できる特殊バージョン）
 *
 * CZipFileが依存するIShellDispatchのZIP展開機能には
 * 拡張子がzipでないアーカイブを解凍できない
 * の制約があるためで作成した。
 * 
 * @param [in] prefix ファイル名の前に付ける3文字の接頭辞。
 * @param [in] extension ファイルの拡張子（.zipを指定する）。
 */
std::filesystem::path GetTempFilePathWithExt(std::wstring_view prefix, std::wstring_view extension)
{
	// 1回だけリトライする
	for (auto n = 0; n <= 1; ++n) {
		// 拡張子指定なし版を呼び出す
		auto tempPath = GetTempFilePath(prefix);

		// 作成された一時ファイルを削除する
		std::filesystem::remove(tempPath);

		tempPath.replace_extension(extension.data());

		if (std::error_code ec; !std::filesystem::exists(tempPath, ec)) {
			return tempPath;
		}
	}

	return {};
}

void extract_zip(
	const std::filesystem::path& zipPath,
	const std::filesystem::path& outDir
)
{
	// 出力先ディレクトリを作成する
	std::filesystem::create_directories(outDir);

	miniz_cpp::zip_file z(zipPath.string());

	for (const auto& name : z.namelist()) {
		const auto outPath = outDir / std::filesystem::path(name);

		// base 配下に収まってるか（../ 脱出対策）
		auto b = std::filesystem::weakly_canonical(outDir);
		auto x = std::filesystem::weakly_canonical(outPath);

		// 文字列比較で prefix 判定（簡易だが実用十分）
		auto& bs = b.native();
		if (auto& xs = x.native(); xs.size() < bs.size() || !xs.starts_with(bs)) {
			throw std::domain_error(std::format("skip dangerous entry: {}", name));
		}

        // ZIP のディレクトリエントリを考慮
        if (!name.empty() && !outPath.has_filename()) {
			std::filesystem::create_directories(outPath);
            continue;
        }

		if (const auto parentDir = outPath.parent_path(); !fexist(parentDir)) {
			std::filesystem::create_directories(parentDir);
		}

        const auto data = z.read(name); // 展開済みバイト列が std::string で返る

        std::ofstream ofs(outPath, std::ios::binary);
        if (!ofs) {
            std::cerr << "failed to open: " << outPath << "\n";
            continue;
        }

		ofs.write(data.data(), static_cast<std::streamsize>(data.size()));
    }
}

void extract_zip_resource(
	WORD id,
	const std::optional<std::filesystem::path>& optOutDir
)
{
	// 一時ファイル名を生成する
	auto tempPath = GetTempFilePath(L"tes");

	// リソースからzipファイルデータを抽出する
	const auto bin = cxx::lock_resource<std::byte>(
		id,
		[] (std::span<const std::byte> resData) {
			return BinarySequence(resData.begin(), resData.end());
		},
		RT_ZIPRES
	);

	// 取得したzipファイルデータを一時ファイルに書き込む
	WriteBinaryToFile(bin, tempPath);
	assert(std::filesystem::exists(tempPath));

	extract_zip(tempPath, optOutDir.value_or(GetIniFileName().remove_filename()));

	// 作成した一時ファイルを削除する
	std::filesystem::remove(tempPath);
}

/*!
 * @brief CZipFIleのテスト
 */
TEST(CZipFile, IsNG)
{
	// IShellDispatchを使うためにOLEを初期化する必要がある
	// このテストでは初期化を忘れた場合の挙動を確認する
	CZipFile cZipFile;
	EXPECT_FALSE(cZipFile.IsOk());

	// この場合、他のメソッドを呼び出すと落ちる。
}

/*!
 * @brief CZipFIleのテスト
 */
TEST(CZipFile, CZipFIle)
{
	// IShellDispatchを使うためにOLEを初期化する
	if (FAILED(::OleInitialize(nullptr))) {
		FAIL();
	}
	else {
		// インスタンス作成時にOLEが初期化されていればIsOkはtrueを返す
		CZipFile cZipFile;
		ASSERT_TRUE(cZipFile.IsOk());

		std::wstring folderName;
		EXPECT_FALSE(cZipFile.ChkPluginDef(L"plugin.def", folderName));

		EXPECT_FALSE(cZipFile.Unzip(L"out"));

		// 一時ファイル名を生成する
		// zipファイルパスの拡張子はzipにしないと動かない。
		auto tempPath = GetTempFilePathWithExt(L"tes", L"zip");

		// リソースからzipファイルデータを抽出して一時ファイルに書き込む
		const auto bin = cxx::lock_resource<std::byte>(
			IDR_ZIPRES1,
			[] (std::span<const std::byte> resData) {
				return BinarySequence(resData.begin(), resData.end());
			},
			RT_ZIPRES
		);
		ASSERT_FALSE(bin.empty());
		ASSERT_TRUE(WriteBinaryToFile(bin, tempPath));
		ASSERT_TRUE(std::filesystem::exists(tempPath));

		// zipファイルパスを設定する
		EXPECT_TRUE(cZipFile.SetZip(tempPath.c_str()));

		// プラグイン設定があるかチェックする
		EXPECT_TRUE(cZipFile.ChkPluginDef(L"plugin.def", folderName));

		// zipファイルを解凍する
		// 展開自体はWindowsの機能なので、展開後パスの存在チェックのみ行う
		const auto dest = std::filesystem::current_path().append(L"unzipped").append(L"");
		std::filesystem::create_directories(dest);
		EXPECT_TRUE(cZipFile.Unzip(dest.c_str()));
		EXPECT_TRUE(std::filesystem::exists(dest / folderName.c_str() / L"plugin.def"));
		std::filesystem::remove_all(dest);

		// 意図的に失敗させる
		EXPECT_FALSE(cZipFile.Unzip(GetExeFileName()));

		// zipファイルパスをクリアする
		EXPECT_TRUE(cZipFile.SetZip(L""));

		// 存在しないzipファイルパスを設定する
		EXPECT_FALSE(cZipFile.SetZip(L"not found"));

		// 作成した一時ファイルを削除する
		std::filesystem::remove(tempPath);
	}

	// OLEをシャットダウンする
	::OleUninitialize();
}
