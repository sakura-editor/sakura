/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_OS_0C5BD7E8_67ED_467C_916F_CCDC1F9A26BF_H_
#define SAKURA_OS_0C5BD7E8_67ED_467C_916F_CCDC1F9A26BF_H_
#pragma once

#include "cxx/type_of_Nth_lambda_arg.hpp"

#include <ObjIdl.h> // LPDATAOBJECT

//クリップボード
bool SetClipboardText( HWND hwnd, const WCHAR* pszText, int nLength ); //!< クリープボードにText形式でコピーする。UNICODE版。nLengthは文字単位。
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat );
HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat );

//	Sep. 10, 2002 genta CWSH.cppからの移動に伴う追加
bool ReadRegistry(HKEY Hive, const WCHAR* Path, const WCHAR* Item, WCHAR* Buffer, unsigned BufferCount);

//	May 01, 2004 genta マルチモニタ対応のデスクトップ領域取得
bool GetMonitorWorkRect(HWND     hWnd, LPRECT prcWork, LPRECT prcMonitor = nullptr);	// 2006.04.21 ryoji パラメータ prcMonitor を追加
bool GetMonitorWorkRect(LPCRECT  prc,  LPRECT prcWork, LPRECT prcMonitor = nullptr);	// 2006.04.21 ryoji
bool GetMonitorWorkRect(POINT    pt,   LPRECT prcWork, LPRECT prcMonitor = nullptr);	// 2006.04.21 ryoji
bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor = nullptr);	// 2006.04.21 ryoji

// 2006.06.17 ryoji
#define PACKVERSION( major, minor ) MAKELONG( minor, major )
DWORD GetComctl32Version();					// Comctl32.dll のバージョン番号を取得						// 2006.06.17 ryoji
BOOL IsVisualStyle();						// 自分が現在ビジュアルスタイル表示状態かどうかを示す		// 2006.06.17 ryoji
void PreventVisualStyle( HWND hWnd );		// 指定ウィンドウでビジュアルスタイルを使わないようにする	// 2006.06.23 ryoji
void MyInitCommonControls();				// コモンコントロールを初期化する							// 2006.06.21 ryoji

/* Wow64 のエミュレーション上で実行しているか判定する */
BOOL IsWow64();

/*!
	@brief Wow64 の ファイルシステムリダイレクションを一時的に無効にして、クラス破棄時に元に戻すクラス
	@note  このクラスを継承しないように final をつける
*/
class CDisableWow64FsRedirect final {
public:
	/*!
		@brief 	コンストラクタで ファイルシステムリダイレクションを無効にする
		@param isOn この引数が TRUE のときに無効化処理を行う
	*/
	CDisableWow64FsRedirect(BOOL isOn);

	/*!
		@brief 	ファイルシステムリダイレクションを元に戻す
	*/
	~CDisableWow64FsRedirect();
	
	// コピー不可 (C++11 で利用可能)
	CDisableWow64FsRedirect(const CDisableWow64FsRedirect&) = delete;
	CDisableWow64FsRedirect& operator = (const CDisableWow64FsRedirect&) = delete;
	
	// ムーブ不可 (C++11 で利用可能)
	CDisableWow64FsRedirect(CDisableWow64FsRedirect&&) = delete;
	CDisableWow64FsRedirect& operator = (CDisableWow64FsRedirect&&) = delete;

private:
	BOOL	m_isSuccess = FALSE;
	PVOID	m_OldValue = nullptr;
};

//カレントディレクトリユーティリティ。
//コンストラクタでカレントディレクトリを保存し、デストラクタでカレントディレクトリを復元するモノ。
//2008.03.01 kobake 作成
class CCurrentDirectoryBackupPoint{
	using Me = CCurrentDirectoryBackupPoint;

public:
	CCurrentDirectoryBackupPoint();
	CCurrentDirectoryBackupPoint(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CCurrentDirectoryBackupPoint(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CCurrentDirectoryBackupPoint();
private:
	WCHAR m_szCurDir[_MAX_PATH];
};

/*!
	@brief PowerShell が利用可能か判定する
*/
BOOL IsPowerShellAvailable(void);

/*!
	@brief IMEのオープン状態を設定する
	@param hWnd 設定対象のウィンドウハンドル
	@param bOpen 設定するオープン状態
	@param pBackup `nullptr` でなければ設定前のオープン状態を取得
	@return	手続きが成功したら true 失敗したら false
*/
BOOL ImeSetOpen(HWND hWnd, BOOL bOpen, BOOL* pBackup);

namespace cxx {

/*!
 * @brief グローバルメモリを RAII で管理するヘルパークラス
 */
class GlobalMemory : public cxx::ResourceHolder<&::GlobalFree> {
private:
	using Base = cxx::ResourceHolder<&::GlobalFree>;
	using Me = GlobalMemory;

public:
	//HGLOBALを指定して構築（メモリ変更は行わない）
	using Base::Base;

	//サイズだけ指定して構築（確保したメモリはゼロクリアされる）
	explicit GlobalMemory(SIZE_T cch) : GlobalMemory(::GlobalAlloc(GHND, cch)) {}

	//HGLOBALを取得する
	HGLOBAL Get() const noexcept { return Base::get(); }

	//グローバルメモリをロックしてデータにアクセスする
	template<typename TAction, typename T = cxx::type_of_Nth_lambda_arg<0, TAction>, typename R = cxx::lambda_traits<TAction>::return_type>
	R Lock(TAction action) const
	{
		const auto hgClip = static_cast<HGLOBAL>(*this);
		if (!hgClip) {
			if constexpr (std::is_same_v<R, void>) {
				return;	//戻り値は返せない
			} else {
				return R{};
			}
		}

		auto pClip = static_cast<T>(::GlobalLock(hgClip));
		if (!pClip) {
			if constexpr (std::is_same_v<R, void>) {
				return;	//戻り値は返せない
			} else {
				return R{};
			}
		}

		using LockedGlobalHolder = cxx::ResourceHolder<&::GlobalUnlock>;
		LockedGlobalHolder locked(hgClip);

		if constexpr (std::is_same_v<R, void>) {
			action(pClip);
		} else {
			const auto cbSize = ::GlobalSize(hgClip);
			return action(pClip, cbSize);
		}
	}
};

/*!
 * @brief グローバルメモリ上の文字列を RAII で管理するヘルパークラス
 */
class GlobalWString : public GlobalMemory {
private:
	using Base = GlobalMemory;
	using Me = GlobalWString;

public:
	//HGLOBALを指定して構築（メモリ変更は行わない）
	using Base::Base;

	//文字数だけ指定して構築（確保したメモリはゼロクリアされる）
	explicit GlobalWString(SIZE_T cch) : GlobalMemory(sizeof(WCHAR) * (cch + 1)) {}

	//文字列を指定して構築（指定した文字列を確保したメモリにコピーする）
	explicit GlobalWString(std::wstring_view text);

	//文字列を指定して更新する
	void SetText(std::wstring_view text) const;

	//格納されている文字列データのコピーを取得する
	std::wstring wstring() const &;
};

/*!
 * @brief グローバルメモリ上の文字列を RAII で管理するヘルパークラス
 */
class GlobalString : public GlobalMemory {
private:
	using Base = GlobalMemory;
	using Me = GlobalString;

public:
	//HGLOBALを指定して構築（メモリ変更は行わない）
	using Base::Base;

	//文字数だけ指定して構築（確保したメモリはゼロクリアされる）
	explicit GlobalString(SIZE_T cch) : GlobalMemory(cch + 1) {}

	//文字列を指定して構築（指定した文字列を確保したメモリにコピーする）
	explicit GlobalString(std::string_view text);

	//文字列を指定して更新する
	void SetText(std::string_view text) const;

	//格納されている文字列データのコピーを取得する
	std::string string() const &;
};

/*!
 * @brief グローバルメモリ上のデータを RAII で管理するヘルパークラス
 */
template<typename T>
class GlobalData : public GlobalMemory {
private:
	using Base = GlobalMemory;
	using Me = GlobalData<T>;

public:
	//HGLOBALを指定して構築（メモリ変更は行わない）
	using Base::Base;

	//データ数だけ指定して構築（確保したメモリはゼロクリアされる）
	explicit GlobalData(int count) : GlobalMemory(count * sizeof(T)) {}

	//1データで構築
	GlobalData() : GlobalData(1) {}

	//データを指定して更新する
	bool SetData(std::span<const T> data) const
	{
		return Lock([data](T* p, size_t cbSize) {
			if (cbSize < std::size(data)) return false;
			std::ranges::copy(data, p);
			return true;
		});
	}

	//データを指定して更新する
	bool SetValue(const T& data) const
	{
		return Lock([data](T* p, size_t cbSize) {
			if (cbSize < sizeof(T)) return false;
			*p = data;
			return true;
		});
	}

	//格納されているデータの個数を取得する
	size_t size() const & {
		const auto cbSize = ::GlobalSize(get());
		return cbSize / sizeof(T);
	}

	//格納されているデータのコピーを取得する
	std::vector<T> data() const & {
		return Lock([](T* p, size_t cbSize) {
			if (cbSize < sizeof(T)) return std::vector<T>();
			return std::vector<T>(p, std::bit_cast<T*>(LPBYTE(p) + cbSize));
		});
	}

	//格納されている値を取得する
	T value() const & {
		return Lock([](T* p, size_t cbSize [[maybe_unused]]) {
			if (cbSize < sizeof(T)) return T();
			return *p;
		});
	}
};

/*!
 * @brief グローバルメモリ上のデータを RAII で管理するヘルパークラス
 */
class GlobalDropFiles : public GlobalMemory {
private:
	using Base = GlobalMemory;
	using Me = GlobalDropFiles;

public:
	//HGLOBALを指定して構築（メモリ変更は行わない）
	using Base::Base;

	//格納されているデータのコピーを取得する
	std::vector<std::filesystem::path> data() const &;
};

/*!
 * @brief グローバルメモリ上の文字列を RAII で管理するヘルパークラス
 */
class GlobalSakura : public GlobalMemory {
private:
	using Base = GlobalMemory;
	using Me = GlobalSakura;

	using size_type = size_t;	//TODO: int32_t に戻す

public:
	//HGLOBALを指定して構築（メモリ変更は行わない）
	using Base::Base;

	//文字列を指定して必要サイズを計算する
	static size_t CalcSize(std::wstring_view text) noexcept;

	//文字列を指定して構築（指定した文字列を確保したメモリにコピーする）
	explicit GlobalSakura(std::wstring_view text);

	//文字列を指定して更新する
	void SetText(std::wstring_view text) const;

	//格納されている文字列データのコピーを取得する
	std::wstring wstring() const &;
};

GlobalDropFiles MakeDropFiles(std::span<const std::filesystem::path> files);

} // namespace cxx

#endif /* SAKURA_OS_0C5BD7E8_67ED_467C_916F_CCDC1F9A26BF_H_ */
