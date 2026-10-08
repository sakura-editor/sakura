/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_
#define SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_
#pragma once

#include "basis/CMyString.h" //CFilePath
#include "basis/primitive.h"
#include "cxx/ResourceHolder.hpp"
#include "util/file.h"

#include <span>
#include <string_view>

namespace cxx {

class NamedFilePointer;

std::span<std::byte> MapViewOfFile(
	_In_ HANDLE hFileMappingObject,
	_In_ DWORD dwDesiredAccess,
	const ULARGE_INTEGER& fileOffset,
	_In_ DWORD dwNumberOfBytesToMap
);

/*!
 * @brief ファイルディスクリプタを閉じるための Deleter
 *
 * std::unique_ptrのテンプレート引数に指定するために用意したもの。
 */
int FdCloseFunc(const int* pFd) noexcept;

/*!
 * @brief OSのファイルハンドルをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class FileHandle : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = FileHandle;

public:
	/*!
	 * @brief ファイルハンドルを作成する
	 *
	 * @param[in] fileName ファイル名
	 * @param[in] dwDesiredAccess アクセス権
	 * @param[in] dwShareMode 共有モード
	 * @param[in] lpSecurityAttributes セキュリティ属性
	 * @param[in] dwCreationDisposition 作成方法
	 * @param[in] dwFlagsAndAttributes ファイル属性と作成フラグ
	 * @param[in] hTemplateFile テンプレートファイルのハンドル
	 */
	static Me CreateFileW(
		std::wstring_view fileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
	);

	/*!
	 * @brief ファイルハンドルを作成する
	 *
	 * @param[in] path ファイル名
	 * @param[in] dwDesiredAccess アクセス権
	 * @param[in] dwShareMode 共有モード
	 * @param[in] lpSecurityAttributes セキュリティ属性
	 * @param[in] dwCreationDisposition 作成方法
	 * @param[in] dwFlagsAndAttributes ファイル属性と作成フラグ
	 * @param[in] hTemplateFile テンプレートファイルのハンドル
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static Me CreateFileW(
		const A& path,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile
	)
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _Path{ path };

		// 文字列を渡すバージョンを呼び出す
		return CreateFileW(
			_Path.str(),
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile
		);
	}

	/*!
	 * @brief 新しいファイルを作る
	 *
	 * @param[in] path ファイル名
	 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ
	 * @return 新しいファイルのハンドル。作成できなかった場合は空のハンドル
	 * @throw std::system_error Windowsがエラーを返したとき
	 */
	static Me CreateNew(
		std::wstring_view fileName,
		_In_opt_ DWORD dwFlagsAndAttributes = 0
	);

	/*!
	 * @brief 新しいファイルを作る
	 *
	 * @param[in] path ファイル名
	 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ
	 * @return 新しいファイルのハンドル。作成できなかった場合は空のハンドル
	 * @throw std::system_error Windowsがエラーを返したとき
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static Me CreateNew(
		const A& path,
		_In_opt_ DWORD dwFlagsAndAttributes = 0
	)
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _Path{ path };

		// 文字列を渡すバージョンを呼び出す
		return CreateNew(
			_Path.str(),
			dwFlagsAndAttributes
		);
	}

	/*!
	 * @brief 存在しているファイルを開く
	 *
	 * @param[in] path ファイル名
	 * @param[in] dwDesiredAccess アクセス権
	 * @param[in] dwShareMode 共有モード
	 * @return 開いたファイルのハンドル。開けなかった場合は空のハンドル
	 * @throw std::system_error Windowsがエラーを返したとき
	 */
	static Me OpenExisting(
		std::wstring_view path,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode
	);

	/*!
	 * @brief 存在しているファイルを開く
	 *
	 * @param[in] path ファイル名
	 * @param[in] dwDesiredAccess アクセス権
	 * @param[in] dwShareMode 共有モード
	 * @return 開いたファイルのハンドル。開けなかった場合は空のハンドル
	 * @throw std::system_error Windowsがエラーを返したとき
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static Me OpenExisting(
		const A& path,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode
	)
	{
		// 引数はNUL終端文字列として扱う
		cxx::NullTerminatedString<WCHAR> _Path{ path };

		// 文字列を渡すバージョンを呼び出す
		return OpenExisting(
			_Path.str(),
			dwDesiredAccess,
			dwShareMode
		);
	}

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;
};

/*!
 * @brief マッピングされたデータをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
template <typename T>
class MappedFileView : public cxx::ResourceHolder<&::UnmapViewOfFile, T>
{
private:
	using Base = cxx::ResourceHolder<&::UnmapViewOfFile, T>;
	using Me = MappedFileView;

public:
	/*!
	 * @brief マッピングされたデータを取得する
	 *
	 * @param[in] hFileMappingObject ファイルマッピングオブジェクトのハンドル
	 * @param[in] dwDesiredAccess 要求するアクセス権
	 * @param[in] maximumSize 最大サイズ
	 */
	static Me MapViewOfFile(
		_In_ HANDLE hFileMappingObject,
		_In_ DWORD dwDesiredAccess,
		const ULARGE_INTEGER& fileOffset = ULARGE_INTEGER{ 0 }
	)
	{
		// マッピングされたデータを返却する
		auto viewOfFile = cxx::MapViewOfFile(
			hFileMappingObject,
			dwDesiredAccess,
			fileOffset,
			sizeof(std::remove_pointer_t<T>)
		);

		return Me{ std::bit_cast<T>(viewOfFile.data()) };
	}

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;
};

/*!
 * @brief OSのファイルマッピングオブジェクトをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class FileMapping : public cxx::ResourceHolder<&::CloseHandle>
{
private:
	using Base = cxx::ResourceHolder<&::CloseHandle>;
	using Me = FileMapping;

public:
	/*!
	 * @brief ファイルマッピングを作る
	 *
	 * @param[in] hFile ファイルハンドル
	 * @param[in] lpFileMappingAttributes セキュリティ属性
	 * @param[in] flProtect 保護属性
	 * @param[in] maximumSize 最大サイズ
	 * @param[in] name ファイルマッピングの名前
	 */
	static Me CreateFileMappingW(
		_In_ HANDLE hFile,
		_In_opt_ LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
		_In_ DWORD flProtect,
		const ULARGE_INTEGER& maximumSize,
		std::wstring_view name
	);

	/*!
	 * @brief ファイルマッピングを開く
	 *
	 * @param[in] dwDesiredAccess 要求するアクセス権
	 * @param[in] bInheritHandle ハンドルを継承するかどうか
	 * @param[in] name ファイルマッピングの名前
	 */
	static Me OpenFileMappingW(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		std::wstring_view name
	);

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	/*!
	 * @brief マッピングされたデータを取得する
	 *
	 * @param[in] dwDesiredAccess 要求するアクセス権
	 * @param[in] fileOffset マップするデータのファイル内オフセット
	 */
	template <typename T>
	MappedFileView<T> MapViewOfFile(
		_In_ DWORD dwDesiredAccess,
		const ULARGE_INTEGER& fileOffset = ULARGE_INTEGER{ 0 }
	) const
	{
		return MappedFileView<T>::MapViewOfFile(
			get(),
			dwDesiredAccess,
			fileOffset
		);
	}
};

/*!
 * @brief Cストリームをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 * fopen() で開いた Cストリーム(FILE*) を RAII っぽく扱えるようにする。
 */
class FilePointer : public cxx::ResourceHolder<&::fclose>
{
private:
	using Base = cxx::ResourceHolder<&::fclose>;
	using Me = FilePointer;

public:
	/*!
	 * @brief 新しいファイルを作って Cストリーム を開く
	 *
	 * @param[in] path 作成するファイルのパス
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	static NamedFilePointer CreateFilePath(
		std::wstring_view path
	);

	/*!
	 * @brief 新しいファイルを作って Cストリーム を開く
	 *
	 * @param[in] path 作成するファイルのパス
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static NamedFilePointer CreateFilePath(
		const A& path
	);

	/*!
	 * @brief 一時ファイルを作成する
	 *
	 * @param[in, opt] optPrefix ファイル名の前に付ける3文字の接頭辞。
	 * @param[in, opt] optTempDir 一時フォルダーのパス。指定しない場合はシステムの一時フォルダーを使う。
	 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ。指定しない場合は FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE を使う。
	 * @param[in, opt] optExt 一時ファイルの拡張子。指定しない場合は "tmp" を使う。
	 */
	static NamedFilePointer CreateTempFile(
		const std::optional<std::wstring>& optPrefix = std::nullopt,
		const std::optional<std::wstring>& optTempDir = std::nullopt,
		DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
		const std::optional<std::wstring>& optExt = std::nullopt
	);

	/*!
	 * @brief OSのファイルハンドルから Cストリーム を開く
	 *
	 * @param[in] hFile OSのファイルハンドル
	 * @param[in] mode _wfdopen() に渡すモード文字列。
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	static Me OpenFileHandle(
		FileHandle&& hFile,
		std::wstring_view mode
	);

	/*!
	 * @briefパスを指定して Cストリーム を開く
	 *
	 * @param[in] path ファイルパス
	 * @param[in] mode fopen() のモード文字列
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	static NamedFilePointer OpenFilePath(
		std::wstring_view path,
		std::wstring_view mode
	);

	/*!
	 * @briefパスを指定して Cストリーム を開く
	 *
	 * @param[in] path ファイルパス
	 * @param[in] mode fopen() のモード文字列
	 * @return 開いたCストリーム。開けなかった場合は無効なCストリーム。
	 */
	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	static NamedFilePointer OpenFilePath(
		const A& path,
		std::wstring_view mode
	);

	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	FilePointer(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	FilePointer(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	virtual ~FilePointer() = default;

	/*!
	 * @brief ストリームからデータを読み込む
	 *
	 * @param[in, out] buffer 読み込むバッファー
	 */
	void read(
		std::string& buffer
	) const;

	/*!
	 * @brief 指定した位置にファイルポインターを移動する
	 *
	 * @param[in] offset 移動先の位置
	 */
	void seek(
		long offset
	) const;

	/*!
	 * @brief ストリームにデータを書き込む
	 *
	 * @param[in] data 書き込むデータ
	 */
	void write(
		std::string_view data
	) const;
};

/*!
 * @brief パス付き Cストリーム をラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 * fopen() で開いた Cストリーム(FILE*) を RAII っぽく扱えるようにする。
 * Cストリームとファイルパスを紐付ける拡張を施したもの。
 */
class NamedFilePointer : public cxx::FilePointer
{
private:
	using Base = cxx::FilePointer;
	using Me = NamedFilePointer;

public:
	/*!
	 * コンストラクタは流用する
	 */
	using Base::Base;

	explicit NamedFilePointer(
		FilePointer&& fp,
		std::wstring_view path
	);

	template <basis::NullTerminatedStringConstructible<WCHAR> A>
		requires (!std::is_same_v<A, std::wstring_view>)
	explicit NamedFilePointer(
		FilePointer&& fp,
		const A& path
	)
		: NamedFilePointer(
			std::move(fp),
			cxx::NullTerminatedString<WCHAR>{ path }.str()
		)
	{
	}

	NamedFilePointer(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	NamedFilePointer(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	std::wstring GetPath() const { return std::wstring{ m_Path.str() }; }

private:
	SFilePath	m_Path;
};

template <basis::NullTerminatedStringConstructible<WCHAR> A>
	requires (!std::is_same_v<A, std::wstring_view>)
NamedFilePointer FilePointer::CreateFilePath(
	const A& path
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> fileName{ path };

	// 文字列を渡すバージョンを呼び出す
	return CreateFilePath(
		fileName.str()
	);
}

template <basis::NullTerminatedStringConstructible<WCHAR> A>
	requires (!std::is_same_v<A, std::wstring_view>)
NamedFilePointer FilePointer::OpenFilePath(
	const A& path,
	std::wstring_view mode
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> fileName{ path };

	// 文字列を渡すバージョンを呼び出す
	return OpenFilePath(
		fileName.str(),
		mode
	);
}

} // namespace cxx

//!ファイルの排他制御モード  2007.10.11 kobake 作成
enum EShareMode{
	SHAREMODE_NOT_EXCLUSIVE,	//!< 排他制御しない
	SHAREMODE_DENY_WRITE,		//!< 他プロセスからの上書きを禁止
	SHAREMODE_DENY_READWRITE,	//!< 他プロセスからの読み書きを禁止
};

class CFile{
	using Me = CFile;

public:
	//コンストラクタ・デストラクタ
	CFile(LPCWSTR pszPath = nullptr);
	CFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CFile();
	//パス
	const CFilePath& GetFilePathClass() const { return m_szFilePath; }
	LPCWSTR GetFilePath() const { return m_szFilePath; }
	//設定
	void SetFilePath(LPCWSTR pszPath){ m_szFilePath.Assign(pszPath); }
	//各種判定
	bool IsFileExist() const;
	bool HasWritablePermission() const;
	bool IsFileWritable() const;
	bool IsFileReadable() const;
	//ロック
	bool FileLock(EShareMode eShareMode, bool bMsg);	//!< ファイルの排他ロック
	void FileUnlock();						//!< ファイルの排他ロック解除
	bool IsFileLocking() const{ return m_hLockedFile!=INVALID_HANDLE_VALUE; }
	EShareMode GetShareMode() const{ return m_nFileShareModeOld; }
	void SetShareMode(EShareMode eShareMode) { m_nFileShareModeOld = eShareMode; }
private:
	CFilePath	m_szFilePath;				//!< ファイルパス
	HANDLE		m_hLockedFile;				//!< ロックしているファイルのハンドル
	EShareMode	m_nFileShareModeOld;		//!< ファイルの排他制御モード
};

#ifdef ENABLE_UNUSED_LEGACY_CODES

/*!
 * @brief Cランタイム「名無しの一時ファイル」
 *
 * 名無しの一時ファイルを作るための機構。
 *
 * @note C4996警告が出るうえ、TOCTOUの危険性があるので、使わないことにする。

 */
class CTmpFile{
	using Me = CTmpFile;

public:
	CTmpFile(){ m_fp = tmpfile(); }
	CTmpFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CTmpFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CTmpFile(){ fclose(m_fp); }
	FILE* GetFilePointer() const{ return m_fp; }
private:
	FILE* m_fp;
};

#endif // ifdef ENABLE_UNUSED_LEGACY_CODES

#endif /* SAKURA_CFILE_53DA3C63_95C0_49D0_9ED1_1C0131493912_H_ */
