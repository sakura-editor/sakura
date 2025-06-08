/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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
#include "pch.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "io/CZipFile.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <locale>
#include <string>

#include "tests1_rc.h"

#define RT_ZIPRES MAKEINTRESOURCE(101)

using BinarySequence = std::basic_string<std::byte>;
using BinarySequenceView = std::basic_string_view<std::byte>;

/*!
	リソースに埋め込まれたデータを取得する
 */
BinarySequence CopyBinaryFromResource(uint16_t nResourceId, LPCWSTR resource_type)
{
	const auto hInst = static_cast<HMODULE>(nullptr);

	const auto hRsrc = ::FindResourceW(hInst, MAKEINTRESOURCE(nResourceId), resource_type);
	if (!hRsrc) {
		throw std::runtime_error("missing resource!");
	}

	// 見つかったリソースを読み込む
	if (const auto hGlobal = ::LoadResource(hInst, hRsrc)) {
		if (uint32_t cbSize = ::SizeofResource(hInst, hRsrc)) {
			// リソースのデータポインタを取得する
			const auto data = static_cast<std::byte*>(::LockResource(hGlobal));

			// リソースデータをコピーする
			BinarySequence ret(data, cbSize);

			// リソースポインタを解放する
			// ※注意：現代のWindows SDKにおいて、UnlockResourceマクロの実装はnop。
			//   この実装では、あえて古代の慣習に従いロック開放をコーディングしてある。
			UnlockResource(hGlobal);

			return ret;
		}
	}

	return {};
}

/*!
	バイナリデータをファイルに書き込む

	@param [in] bin バイナリデータ
	@param [in] path 書き込み先ファイルパス
 */
bool WriteBinaryToFile(BinarySequenceView bin, std::filesystem::path path)
{
	if (bin.length()) {
		using std::ios;

		// 内部的なストリームインスタンスを用意する
		// std::byteでパラメータ化したstd::basic_ofstreamだとMinGWビルドが動作しないので、
		// あえて標準の1バイト実装を使う
		std::ofstream os;
		os.open(path.c_str(), ios::binary | ios::trunc);

		if (os) {
			os.write(reinterpret_cast<const char*>(bin.data()), bin.length());
			return true;
		}
	}

	return false;
}

/*!
	新しいテンポラリファイルパスを生成する
	Windows APIが生成したパスを開いて閉じるため、呼ぶとファイルが生成される

	生成されるパスの形式は以下の通り。
	C:\Users\berryzplus\AppData\Local\Temp\tesC85A.tmp

	@param [in] prefix ファイル名の前に付ける3文字の接頭辞。
 */
std::filesystem::path GetTempFilePath(std::wstring_view prefix)
{
	// 一時フォルダーのパスを取得する
	const std::wstring tempDir = std::filesystem::temp_directory_path();

	// パス生成に必要なバッファを確保する
	// （一時フォルダーのパス＋接頭辞(3文字)＋4桁の16進数＋拡張子＋NUL終端）
	std::wstring buf(tempDir.length() + 3 + 4 + 4 + 1, L'\0');

	// Windows API関数を呼び出す。
	// （オーバーフローしないので、エラーチェック省略）
	constexpr uint16_t uUnique = 0;
	::GetTempFileNameW(tempDir.c_str(), prefix.data(), uUnique, buf.data());

	return buf.c_str();
}

/*!
	新しいテンポラリファイルパスを生成する
	（拡張子を指定できるオーバーロード版）

	@param [in] prefix ファイル名の前に付ける3文字の接頭辞。
 */
std::filesystem::path GetTempFilePath(std::wstring_view prefix, std::wstring_view extension)
{
	// 1回だけリトライする
	for (auto n = 0; n <= 1; ++n) {
		// 拡張子指定なし版を呼び出す
		auto tempPath = GetTempFilePath(prefix);

		// 作成された一時ファイルを削除する
		std::filesystem::remove(tempPath);

		tempPath.replace_extension(extension.data());

		if (std::error_code ec; !std::filesystem::exists(tempPath, ec)); {
			return tempPath;
		}
	}

	return {};
}

/*!
 * @brief CZipFIleのテスト
 */
TEST(CZipFIle, IsNG)
{
	// IShellDispatchを使うためにOLEを初期化する必要がある
	// このテストでは初期化を忘れた場合の挙動を確認する
	CZipFile cZipFile;
	ASSERT_FALSE(cZipFile.IsOk());

	// この場合、他のメソッドを呼び出すと落ちる。
}

/*!
 * @brief CZipFIleのテスト
 */
TEST(CZipFIle, CZipFIle)
{
	// IShellDispatchを使うためにOLEを初期化する
	if (FAILED(::OleInitialize(nullptr))) {
		FAIL();
	}
	else {
		// インスタンス作成時にOLEが初期化されていればIsOkはtrueを返す
		CZipFile cZipFile;
		ASSERT_TRUE(cZipFile.IsOk());

		// 一時ファイル名を生成する
		// zipファイルパスの拡張子はzipにしないと動かない。
		auto tempPath = GetTempFilePath(L"tes", L"zip");

		// リソースからzipファイルデータを抽出して一時ファイルに書き込む
		const auto bin = CopyBinaryFromResource(IDR_ZIPRES1, RT_ZIPRES);
		ASSERT_FALSE(bin.empty());
		ASSERT_TRUE(WriteBinaryToFile(bin, tempPath));
		ASSERT_TRUE(std::filesystem::exists(tempPath));

		// zipファイルパスを設定する
		ASSERT_TRUE(cZipFile.SetZip(tempPath.c_str()));

		// Azure PipelinesとGitHub Actionsで機能しないため、以下テスト省略。
		//// プラグイン設定があるかチェックする
		//std::wstring folderName;
		//ASSERT_TRUE(cZipFile.ChkPluginDef(L"plugin.def", folderName));
		std::wstring folderName = L"test-plugin";

		// zipファイルを解凍する
		// 展開自体はWindowsの機能なので、展開後パスの存在チェックのみ行う
		const auto dest = std::filesystem::current_path().append(L"unzipped").append(L"");
		std::filesystem::create_directories(dest);
		ASSERT_TRUE(cZipFile.Unzip(dest.c_str()));
		ASSERT_TRUE(std::filesystem::exists(dest / folderName.c_str() / L"plugin.def"));
		std::filesystem::remove_all(dest);

		// zipファイルパスをクリアする
		ASSERT_TRUE(cZipFile.SetZip(L""));

		// 存在しないzipファイルパスを設定する
		ASSERT_FALSE(cZipFile.SetZip(L"not found"));

		// 作成した一時ファイルを削除する
		std::filesystem::remove(tempPath);
	}

	// OLEをシャットダウンする
	::OleUninitialize();
}
