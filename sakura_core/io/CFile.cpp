/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "io/CFile.h"

#include "window/CEditWnd.h" // 変更予定
#include "CSelectLang.h"

#include <sys/types.h>
#include <sys/stat.h>	// _fstat で必要。先に sys/types.h をincludeする必要がある。

#include <fcntl.h>
#include <io.h>

#include <array>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace cxx {

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
 * @brief 新しいファイルを作る
 *
 * 指定されたパスに新しいファイルを作成します。
 * 既に存在している場合、戻り値は空になります。
 *
 * @param[in] path ファイル名
 * @param[in, opt] dwFlagsAndAttributes ファイル属性と作成フラグ
 * @return 新しいファイルのハンドル。作成できなかった場合は空のハンドル
 * @throw std::system_error Windowsがエラーを返したとき
 */
/* static */ FileHandle FileHandle::CreateNew(
	std::wstring_view path,
	_In_opt_ DWORD dwFlagsAndAttributes
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Path{ path };

	// 新規作成なので書き込み権限を要求する
	DWORD dwDesiredAccess = GENERIC_WRITE;

	// 書き込み共有は許可する
	DWORD dwShareMode	  = FILE_SHARE_WRITE;

	// 一時ファイル属性が付いていたら読めるようにしておく
	if (dwFlagsAndAttributes & FILE_ATTRIBUTE_TEMPORARY) {
		dwDesiredAccess |= GENERIC_READ;
		dwShareMode		|= FILE_SHARE_READ;
	}

	// 「閉じたら削除」フラグが立っていたら削除できるようにしておく
	if (dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE) {
		dwDesiredAccess |= DELETE;
		dwShareMode		|= FILE_SHARE_DELETE;
	}

	// OSのファイルハンドルを返却する
	FileHandle hFile{};

	// ファイルを作成する
	try {
		hFile = CreateFileW(
			_Path.str(),
			dwDesiredAccess,
			dwShareMode,
			nullptr,
			CREATE_NEW,
			dwFlagsAndAttributes,
			nullptr
		);
	}
	// エラーが発生した場合
	catch (const std::system_error& e) {
		// エラーコードが「既に存在」を示す場合以外はそのまま上位に投げる
		if (ERROR_FILE_EXISTS != e.code().value() &&
			ERROR_ALREADY_EXISTS != e.code().value())
		{
			throw;
		}
	}

	return hFile;
}

/*!
 * @brief 存在しているファイルを開く
 *
 * 指定されたパスに存在するファイルを開きます。
 *
 * @param[in] path ファイル名
 * @param[in] dwDesiredAccess アクセス権
 * @param[in] dwShareMode 共有モード
 * @return 開いたファイルのハンドル。開けなかった場合は空のハンドル
 * @throw std::system_error Windowsがエラーを返したとき
 */
/* static */ FileHandle FileHandle::OpenExisting(
	std::wstring_view path,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode
)
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _Path{ path };

	// OSのファイルハンドルを返却する
	FileHandle hFile{};

	// 存在しているファイルを開く
	try {
		hFile = CreateFileW(
			_Path.str(),
			dwDesiredAccess,
			dwShareMode,
			nullptr,
			OPEN_EXISTING,
			0L,				// 属性は指定しない
			nullptr
		);
	}
	// エラーが発生した場合
	catch (const std::system_error& e) {
		// エラーコードがアクセス拒否以外のエラーはそのまま上位に投げる
		if (ERROR_SHARING_VIOLATION != e.code().value() &&
			ERROR_ACCESS_DENIED != e.code().value())
		{
			throw;
		}
	}

	return hFile;
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
	// '+' が指定されず、'w' が指定されていたら書き込み専用モード
	else if (std::wstring_view::npos != mode.find(L"w")) flags |= _O_WRONLY;

	// 'a' が指定されていたら追記モード
	const auto appendMode = std::wstring_view::npos != mode.find(L"a");
	if (appendMode) flags |= _O_APPEND;

	// 'b' が指定されていたら追記モード
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
	}

	// 書き込みテスト
	if (fp &&	// ファイルを開けている
		uint32_t(flags) & _O_RDWR &&	// 読み書き両用モード
		!truncMode &&	// truncateしないモード
		!appendMode)	// 追記モードではない
	{
		// 書き込みテスト用のデータ（なんでもよい）
		constexpr std::string_view writeTestData = "test";

		// ファイルサイズを取得する
		const auto fileSize = ::_filelengthi64(fd);
		if (fileSize < 0) return FilePointer{};

		// 扱うファイルは32bitに収まるサイズに制限する
		if (std::numeric_limits<uint32_t>::max() <= fileSize) throw std::overflow_error("too large file.");
			
		// ファイルデータを壊さないためのバックアップを格納するバッファー
		std::string backupData(std::min<size_t>(writeTestData.size(), fileSize), '\0');

		// ファイルが空でない場合
		if (0 < fileSize) {
			// ファイルデータを読み込んでバックアップする
			fp.read(backupData);

			// ファイルポインタを先頭に戻す
			fp.seek(0);
		}

		// 書き込み権限を調べるため、実際に書き込む（権限がなければエラーになる）
		fp.write(writeTestData);

		// 書き込んだデータをフラッシュする
		fp.seek(0);

		// 書き込みテスト前のデータを復元する
		if (!backupData.empty()) {
			// バックアップしておいたデータを書き戻す
			fp.write(backupData);

			// 書き込んだデータをフラッシュする
			fp.seek(0);
		}

		// 書き込みテストでファイルが大きくなった場合は元のサイズに戻す
		if (fileSize < std::ssize(writeTestData)) {
			::_chsize_s(fd, fileSize);
		}
	}

	return fp;
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

} // namespace cxx

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CFile::CFile(LPCWSTR pszPath)
: m_hLockedFile( INVALID_HANDLE_VALUE )
, m_nFileShareModeOld( SHAREMODE_NOT_EXCLUSIVE )
{
	if(pszPath){
		SetFilePath(pszPath);
	}
}

CFile::~CFile()
{
	FileUnlock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         各種判定                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFile::IsFileExist() const
{
	return fexist(GetFilePath());
}

bool CFile::HasWritablePermission() const
{
	return -1 != _waccess( GetFilePath(), 2 );
}

bool CFile::IsFileWritable() const
{
	//書き込めるか検査
	// Note. 他のプロセスが明示的に書き込み禁止しているかどうか
	//       ⇒ GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE でチェックする
	//          実際のファイル保存もこれと等価な _wfopen の L"wb" を使用している
	HANDLE hFile = CreateFile(
		this->GetFilePath(),			//ファイル名
		GENERIC_WRITE,					//書きモード
		FILE_SHARE_READ | FILE_SHARE_WRITE,	//読み書き共有
		nullptr,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		nullptr							//テンプレート無し
	);
	if(hFile==INVALID_HANDLE_VALUE){
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool CFile::IsFileReadable() const
{
	HANDLE hTest = CreateFile(
		this->GetFilePath(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		nullptr
	);
	if(hTest==INVALID_HANDLE_VALUE){
		// 読み込みアクセス権がない
		return false;
	}
	CloseHandle( hTest );
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ロック                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ファイルの排他ロック解除
void CFile::FileUnlock()
{
	//クローズ
	if( m_hLockedFile != INVALID_HANDLE_VALUE ){
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = INVALID_HANDLE_VALUE;
	}
}

//! ファイルの排他ロック
bool CFile::FileLock( EShareMode eShareMode, bool bMsg )
{
	// ロック解除
	FileUnlock();

	// ファイルの存在チェック
	if( !this->IsFileExist() ){
		return false;
	}

	// モード設定
	if(eShareMode==SHAREMODE_NOT_EXCLUSIVE)return true;
	
	//フラグ
	DWORD dwShareMode=0;
	switch(eShareMode){
	case SHAREMODE_NOT_EXCLUSIVE:	return true;										break; //排他制御無し
	case SHAREMODE_DENY_READWRITE:	dwShareMode = 0;									break; //読み書き禁止→共有無し
	case SHAREMODE_DENY_WRITE:		dwShareMode = FILE_SHARE_READ;						break; //書き込み禁止→読み込みのみ認める
	default:						dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;	break; //禁止事項なし→読み書き共に認める
	}

	//オープン
	m_hLockedFile = CreateFile(
		this->GetFilePath(),			//ファイル名
		GENERIC_READ,					//読み書きタイプ
		dwShareMode,					//共有モード
		nullptr,							//既定のセキュリティ記述子
		OPEN_EXISTING,					//ファイルが存在しなければ失敗
		FILE_ATTRIBUTE_NORMAL,			//特に属性は指定しない
		nullptr							//テンプレート無し
	);

	//結果
	if( INVALID_HANDLE_VALUE == m_hLockedFile && bMsg ){
		const WCHAR*	pszMode;
		switch( eShareMode ){
		case SHAREMODE_DENY_READWRITE:	pszMode = LS(STR_EXCLU_DENY_READWRITE); break;
		case SHAREMODE_DENY_WRITE:		pszMode = LS(STR_EXCLU_DENY_WRITE); break;
		default:						pszMode = LS(STR_EXCLU_UNDEFINED); break;
		}
		TopWarningMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_FILE_LOCK_ERR),
			GetFilePathClass().IsValidPath() ? GetFilePath() : LS(STR_NO_TITLE1),
			pszMode
		);
		return false;
	}

	return true;
}
