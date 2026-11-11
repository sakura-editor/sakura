/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/file/FilePointer.hpp"

#include "util/file.h"
#include "util/file/NamedFilePointer.hpp"
#include "util/os/Bcrypt.hpp"
#include "util/os/ntstatus_category.hpp"

#include <sys/types.h>
#include <sys/stat.h>	// _fstat で必要。先に sys/types.h をincludeする必要がある。

#include <fcntl.h>
#include <io.h>

#include <algorithm>
#include <array>
#include <exception>
#include <stdexcept>
#include <string_view>
#include <system_error>

#pragma comment(lib, "Bcrypt.lib")

namespace cxx {

std::wstring GetTempPath2W();

template <typename T>
[[nodiscard]] NTSTATUS BCryptGenRandom(
	_In_opt_ BCRYPT_ALG_HANDLE hAlgorithm,
	std::span<T> buffer,
	_In_ ULONG dwFlags
)
{
	return Bcrypt::getInstance()->BCryptGenRandom(
		hAlgorithm,
		PUCHAR(std::data(buffer)),
		ULONG(buffer.size_bytes()),
		dwFlags
	);
}

uint16_t GenerateRandom16()
{
	uint16_t value{};

	if (const auto status = cxx::BCryptGenRandom(
		nullptr,
		std::span(&value, 1),
		BCRYPT_USE_SYSTEM_PREFERRED_RNG
	);
		!BCRYPT_SUCCESS(status))
	{
		// システムエラー例外を発生させる
		throw std::system_error(
			int(status),
			ntstatus_category(),
			"BCryptGenRandom failed"
		);
	}

	return value;
}

/*!
 * @brief ファイルディスクリプタを閉じるための Deleter
 *
 * std::unique_ptrのテンプレート引数に指定するために用意したもの。
 */
int FdCloseFunc(
	const int* pFd
) noexcept
{
	// ファイルディスクリプタを閉じる
	return ::_close(*pFd);
}

/*!
 * @brief ファイルサイズを変更する
 *
 * 指定されたファイルディスクリプタのファイルサイズを変更します。
 *
 * @param[in] fd ファイルディスクリプタ
 * @param[in] fileSize 新しいファイルサイズ
 * @throw std::system_error ファイルサイズの変更に失敗したとき
 */
void resize_file(int fd, int64_t fileSize)
{
	if (const auto ret = ::_chsize_s(fd, fileSize);
		0 == ret)
	{
		return;	// 成功した
	}

	// 失敗したら例外を投げる
	throw std::system_error(
		std::make_error_code(std::errc::io_error),
		"Failed to change file size"
	);
}

/*!
 * @brief 新しいファイルを作って Cストリーム を開く
 *
 * 指定されたパスに新しいファイルを作成します。
 * 既に存在している場合、戻り値は空になります。
 *
 * @param[in] path 作成するファイルのパス
 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
 */
/* static */ NamedFilePointer FilePointer::CreateFilePath(
	std::wstring_view path
)
{
	// ファイルポインターを返却する
	FilePointer fp;

	// ファイルを作成する
	try {
		// OSのファイルハンドルを格納するスマートポインター
		FileHandle hFile{};

		// 新しいファイルを作成する
		hFile = FileHandle::CreateNew(
			path,
			0
		);

		// OSのファイルハンドルを作成できた場合
		if (hFile) {
			// OSのファイルハンドルから Cストリーム を開く
			fp = OpenFileHandle(std::move(hFile), L"wb");
		}
	}
	// エラーが発生した場合
	catch (const std::system_error&) {
		// 作成できなかった
		fp = nullptr;
	}

	// Cストリームとパスを紐付ける
	return NamedFilePointer{ std::move(fp), path };
}

/*!
 * @brief 一時ファイルを作成する
 *
 * ファイルパスを連携してプロセス間でデータを共有するための仕組み。
 *
 * 一時ファイルの Time Of Check/Time Of Use (TOCTOU) 脆弱性を避けるため、
 * 「ファイル作成」をもってチェックとし、開いたファイルを「そのまま使う」ようにする。
 *
 * 生成されるパスの形式は以下の通り。
 * C:\Users\berryzplus\AppData\Local\Temp\tesC85A.tmp
 *
 * @param[in, opt] optPrefix ファイル名の前に付ける3文字の接頭辞。
 * @param[in, opt] optTempDir 一時フォルダーのパス。指定しない場合はシステムの一時フォルダーを使う。
 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ。指定しない場合は FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE を使う。
 * @param[in, opt] optExt 一時ファイルの拡張子。指定しない場合は "tmp" を使う。
 */
/* static */ NamedFilePointer FilePointer::CreateTempFile(
	const std::optional<std::wstring>& optPrefix,
	const std::optional<std::wstring>& optTempDir,
	DWORD dwFlagsAndAttributes,
	const std::optional<std::wstring>& optExt
)
{
	// 一時ファイルの置き場所を解決する
	std::filesystem::path tempDir{ optTempDir.value_or(L"") };
	if (std::error_code ec;
		tempDir.empty() ||
		!std::filesystem::exists(tempDir, ec) ||
		!std::filesystem::is_directory(tempDir, ec))
	{
		// 指定フォルダがないときは一時ディレクトリパスを取得する
		tempDir = cxx::GetTempPath2W();
	}

	// 末尾がパス区切り文字で終わっていたら取り除く
	if (const auto& dir = tempDir.native();
		dir.ends_with(LR"(\)") || dir.ends_with(L"/"))
	{
		tempDir = tempDir.parent_path();
	}

	// 一時ファイルの接頭辞を取得する
	const auto prefix = optPrefix.value_or(GetExeFileName().stem().native());
	if (prefix.empty()) throw std::invalid_argument("prefix is required.");
	if (std::wstring::npos != prefix.find_first_of(LR"(/\)"))  throw std::invalid_argument("prefix should not contain path separators.");

	// 一時ファイルの拡張子を取得する
	const auto ext = optExt.value_or(L".tmp");
	if (ext.empty()) throw std::invalid_argument("ext is required.");
	if (ext[0] != L'.') throw std::invalid_argument("ext must be start with '.'.");

	// ファイルポインターを返却する
	FilePointer fp{};

	// 一時ファイルパスを格納する
	std::filesystem::path path{};

	// 一時ファイルを作成する
	try {
		// 一時ファイルを作成できるまでループ（上限3回。）
		for (int i = 0; i < 3; ++i) {
			// 一時ファイルパスを組み立てる
			path = tempDir / std::format(L"{:s}{:x}{:s}", prefix, cxx::GenerateRandom16(), ext);

			// ファイルハンドルをスマートポインターに入れる
			FileHandle hFile{};

			// 一時ファイルを作成する
			hFile = FileHandle::CreateNew(
				path,
				dwFlagsAndAttributes
			);

			// OSのファイルハンドルを作成できた場合
			if (hFile) {
				// OSのファイルハンドルから Cストリーム を開く
				fp = OpenFileHandle(std::move(hFile), L"wb");
			}

			// Cストリームを作成できた場合はループを抜ける
			if (fp) break;
		}
	}
	// エラーが発生した場合
	catch (const std::system_error&) {
		// 作成できなかった
		fp = nullptr;
	}

	// Cストリームとパスを紐付ける
	return NamedFilePointer{ std::move(fp), path };
}

/*!
 * @brief OSのファイルハンドルから Cストリーム を開く
 *
 * @param[in] hFile OSのファイルハンドル
 * @param[in] mode _wfdopen() に渡すモード文字列。
 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
 */
/* static */ FilePointer FilePointer::OpenFileHandle(
	FileHandle&& hFile,
	std::wstring_view mode
)
{
	// ハンドルの所有権を受け取る
	auto handle{ std::move(hFile) };

	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Mode{ mode };

	// モードを省略すると fdopen がクラッシュするので例外で弾く
	if (mode.empty()) throw std::invalid_argument("missing mode");

	// モードをフラグに変換する。
	int flags = _O_RDONLY;

	// '+' が指定されていたら読み書き両用モード
	if (std::wstring_view::npos != mode.find(L"+"))
	{
		flags |= _O_RDWR;
	}
	// '+' が指定されず、'a' か 'w' が指定されていたら書き込み専用モード
	else if (std::wstring_view::npos != mode.find_first_of(L"aw")) flags |= _O_WRONLY;

	// 'a' が指定されていたら追記モード
	const auto appendMode = std::wstring_view::npos != mode.find(L"a");
	if (appendMode) flags |= _O_APPEND;

	// 'b' が指定されていたらバイナリモード
	if (std::wstring_view::npos != mode.find(L"b"))
	{
		flags |= _O_BINARY;
	}

	// 古いMSVCランタイムのテキストモードは扱いが難しいので「利用しない方針」になっている。
	if (std::wstring_view::npos != mode.find(L"t")) throw std::invalid_argument("DO NOT USE text mode.");

	// 'w' が指定されていたらファイルを空にするモード
	const auto truncMode = std::wstring_view::npos != mode.find(L"w");

	// OSのファイルハンドルからファイル記述子を開く
	auto fd = ::_open_osfhandle(
		intptr_t(handle.get()),
		flags
	);

	// 失敗した場合、fdは -1 になる
	if (-1 == fd) {
		return FilePointer{};	// 開けなかった
	}

	// _open_osfhandle が成功すると hFileの所有権 は Cランタイム に移る。
	//   → fd を fclose() すると
	//      Cランタイムが CloseHandle() して OSのファイルハンドルも閉じる。
	//      自分で CloseHandle() すると壊れるので注意。

	// OSのファイルハンドルをスマートポインタから解放する
	handle.release();

	// fd を閉じるためのスマートポインターを作る
	using FdCloser = std::unique_ptr<int, decltype(&FdCloseFunc)>;
	FdCloser fdCloser{ &fd, &FdCloseFunc };

	// fdからCストリームを開く
	auto fp = FilePointer{ ::_wfdopen(fd, _Mode.c_str()) };

	// 成功した場合、fdの所有権 は Cランタイム に移る。
	if (fp) {
		// 所有権が移ったので、スマートポインターから解放する
		fdCloser.release();

		// ファイルを空にするモードの場合、ファイルを空にする
		if (truncMode) {
			// ファイルディスクリプタを使ってファイルサイズを0にする
			cxx::resize_file(fd, 0);
		}
	}

	// 書き込みテスト
	if (fp &&	// ファイルを開けている
		uint32_t(flags) & _O_RDWR &&	// 読み書き両用モード
		!truncMode &&	// truncateしないモード
		!appendMode)	// 追記モードではない
	{
		// ファイルに書き込めるかどうかをチェックする
		fp.check_write_permissions();
	}

	return fp;
}

/*!
 * @brief ファイルに書き込めるかどうかをチェックする
 *
 * Windowsではアクセス権があっても書き込めないことがある。
 * なので、実際に書き込んでチェックする。
 * チェック後は、内容・サイズ・タイムスタンプの復元を試みる。
 * 
 * @throw std::system_error ファイルサイズが大きすぎるとき
 * @throw std::system_error 書き込みテストに失敗したとき
 */
void FilePointer::check_write_permissions() const
{
	// 書き込みテスト用のデータ（なんでもよい）
	constexpr std::string_view writeTestData = "test";

	// ファイルディスクリプタを取得する
	const auto fd = ::_fileno(get());
	if (-1 == fd) throw std::system_error(std::make_error_code(std::errc::io_error), "Failed to get file descriptor");

	// ファイルサイズを取得する
	const auto fileSize = ::_filelengthi64(fd);
	if (fileSize < 0) throw std::system_error(std::make_error_code(std::errc::io_error), "Failed to get file size");

	// 扱うファイルは32bitに収まるサイズに制限する
	if (std::numeric_limits<uint32_t>::max() <= fileSize) throw std::system_error(std::make_error_code(std::errc::file_too_large), "too large file.");

	// ファイルハンドルを取得する
	FileHandle hFile{ HANDLE(::_get_osfhandle(fd))};

	// ファイルのタイムスタンプを取得する
	bool timestampStored = false;
	FILETIME creationTime{};
	FILETIME lastAccessTime{};
	FILETIME lastWriteTime{};
	try {
		hFile.GetFileTime(&creationTime, &lastAccessTime, &lastWriteTime);
		timestampStored = true;

		// 副作用を防ぐため、一旦解放してしまう
		hFile.release();
	}
	catch (const std::system_error&) {
		// ファイルタイムの取得に失敗しても続行する
		hFile.release();
	}

	// ファイルデータを壊さないためのバックアップを格納するバッファー
	std::string backupData(std::min<size_t>(writeTestData.size(), fileSize), '\0');

	// ファイルが空でない場合、バックアップを作る
	if (0 < fileSize) {
		// ファイルポインタを先頭に戻す
		seek(0);

		// ファイルデータを読み込んでバックアップする
		read(backupData);
	}

	// 最初に発生したエラーを保持する変数
	std::exception_ptr error;

	// 例外を捕捉して、最初の例外を保持するラムダ式
	// 書き込みテストが失敗した場合も、可能な限り元の状態に戻す
	const auto tryOperation = [&error, this](auto&& operation) {
		// 処理の成否
		bool result = false;
		try {
			operation();

			result = true;	// 成功した
		}
		// エラーが発生した場合、最初のエラーを保持する
		catch (const std::system_error&) {
			// 最初のエラーをまだ記録していない場合、記録する
			if (!error) error = std::current_exception();

			// Cストリームのエラー状態をクリアする
			std::clearerr(get());
		}
		return result;
	};

	// 書き込み権限を調べるため、実際に書き込む（権限がなければエラーになる）
	tryOperation([this, writeTestData] {
		// ファイルポインタを先頭に戻す
		seek(0);

		// データを書き込む
		write(writeTestData);

		// 書き込んだデータをフラッシュする
		seek(0);
	});

	// 書き込みテスト前のデータを復元する
	if (!backupData.empty())
	{
		// ファイルポインタを先頭に戻す
		tryOperation([this] { seek(0); });

		// バックアップしたデータを書き戻す
		tryOperation([this, backupData] { write(backupData); });

		// 書き戻したデータをフラッシュする
		tryOperation([this] { seek(0); });
	}

	// 書き込みテストでファイルが大きくなった場合は元のサイズに戻す
	if (fileSize < std::ssize(writeTestData)) {
		tryOperation([fd, fileSize] { cxx::resize_file(fd, fileSize); });
	}

	if (timestampStored) {
		// 再割り当てする
		hFile = FileHandle{ HANDLE(::_get_osfhandle(fd))};

		// ファイルのタイムスタンプを元に戻す
		tryOperation([&hFile, creationTime, lastAccessTime, lastWriteTime] {
			hFile.SetFileTime(&creationTime, &lastAccessTime, &lastWriteTime);
		});

		// ファイルハンドルをスマートポインタから解放する
		hFile.release();
	}

	// エラーが発生していた場合、最初のエラーを re-throw する
	if (error)
	{
		std::rethrow_exception(error);
	}
}

/*!
 * @brief ストリームからデータを読み込む
 *
 * @param[in, out] buffer 読み込むバッファー
 */
void FilePointer::read(
	std::string& buffer
) const
{
	// bufferなしで呼び出す状況は想定しない
	if (buffer.empty()) throw std::invalid_argument("buffer is required.");

	// ストリームにデータを読み込む
	if (const auto read = std::fread(buffer.data(), 1, buffer.size(), get());
		read == buffer.size())
	{
		return;	// 読み込めた
	}

	// 失敗したら例外を投げる
	throw std::system_error(
		std::make_error_code(std::errc::io_error),
		"Failed to read file"
	);
}

/*!
 * @brief 指定した位置にファイルポインターを移動する
 *
 * @param[in] offset 移動先の位置
 */
void FilePointer::seek(
	long offset
) const
{
	// 指定した位置にファイルポインターを移動する
	if (0 == std::fseek(get(), offset, SEEK_SET))
	{
		return;	// シークできた
	}

	// 失敗したら例外を投げる
	throw std::system_error(
		std::make_error_code(std::errc::io_error),
		"Failed to seek file"
	);
}

/*!
 * @brief ストリームにデータを書き込む
 *
 * @param[in] data 書き込むデータ
 */
void FilePointer::write(
	std::string_view data
) const
{
	// dataなしで呼び出す状況は想定しない
	if (data.empty()) throw std::invalid_argument("data is required.");

	// ストリームにデータを書き込む
	if (const auto written = std::fwrite(data.data(), 1, data.size(), get());
		written == data.size())
	{
		return;	// 書き込めた
	}

	// 失敗したら例外を投げる
	throw std::system_error(
		std::make_error_code(std::errc::io_error),
		"Failed to write file"
	);
}

/*!
 * @briefパスを指定して Cストリーム を開く
 *
 * @param[in] path ファイルパス
 * @param[in] mode fopen() のモード文字列
 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
 */
/* static */ NamedFilePointer FilePointer::OpenFilePath(
	std::wstring_view path,
	std::wstring_view mode
)
{
	// 入力元はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Path{ path };
	cxx::NullTerminatedString<WCHAR> _Mode{ mode };

	// パスは必須だが、空で呼んで失敗させる既存コードがあるのでエラーにはしない。
	if (path.empty()) return NamedFilePointer{};

	// モードも必須。省略すると fopen で落ちる。
	if (mode.empty()) throw std::invalid_argument("missing mode");

	// 共有メモリに格納する都合、パス長には制限がある。
	if (SFilePath::size() <= path.length()) {
		// この例外が出る場合、使い方が誤っているので、呼出元を修正すること。
		throw std::system_error(
			std::make_error_code(std::errc::filename_too_long),
			"fileName is too long."
		);
	}

	// まずは fopen で開いてみる
	if (FILE* nativeFp = nullptr;
		0 == ::_wfopen_s(
			&nativeFp,
			_Path.c_str(),
			_Mode.c_str()
		))
	{
		// 通常ファイルはここで開けるはず
		return NamedFilePointer{ FilePointer{ nativeFp }, path };
	}

	// OSファイルハンドルを開くときに指定するフラグ
	DWORD dwDesiredAccess = GENERIC_READ;
	DWORD dwShareMode = FILE_SHARE_READ;

	// 書き込み可能モードなら反映する
	if (std::wstring_view::npos != mode.find_first_of(L"aw+"))
	{
		dwDesiredAccess |= GENERIC_WRITE;
		dwShareMode |= FILE_SHARE_WRITE;
	}

	// ファイルポインターを返却する
	FilePointer fp;

	// OSのファイルハンドルを開く
	try {
		// ファイルハンドルをスマートポインターに入れる
		FileHandle hFileHolder{};

		// 存在しているファイルを開く
		hFileHolder = FileHandle::OpenExisting(
			path,
			dwDesiredAccess,
			dwShareMode
		);

		// 権限エラーで開けなかった場合
		if (!hFileHolder) {
			// 削除権限を要求してリトライする
			hFileHolder = FileHandle::OpenExisting(
				path,
				dwDesiredAccess | DELETE,
				dwShareMode | FILE_SHARE_DELETE
			);
		}

		// OSのファイルハンドルを作成できた場合
		if (hFileHolder) {
			// OSのファイルハンドルから Cストリーム を開く
			fp = OpenFileHandle(std::move(hFileHolder), mode);
		}
	}
	// エラーが発生した場合
	catch (const std::system_error&) {
		// 開けなかった
		fp = nullptr;
	}

	// Cストリームとパスを紐付ける
	return NamedFilePointer{ std::move(fp), path };
}
} // namespace cxx
