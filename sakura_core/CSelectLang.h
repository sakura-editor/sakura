/*!	@file
	@brief 各国語メッセージリソース対応

	@author nasukoji
	@date 2011.04.10	新規作成
*/
/*
	Copyright (C) 2011, nasukoji
	Copyright (C) 2018-2025, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CSELECTLANG_657416B2_2B3D_455C_AC28_8B86244F5F83_H_
#define SAKURA_CSELECTLANG_657416B2_2B3D_455C_AC28_8B86244F5F83_H_
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "cxx/load_string.hpp"

#include "sakura_rc.h"

/*!
 * 言語リリースDLL選択クラス
 *
 * 英語対応のために作成されたものと思われる
 * 完全オリジナルな言語リソースDLLロード実装。
 *
 * @note Windows標準のMUIに移行するまでは削除できない。
 */
class CSelectLang
{
private:
	using ResourceDllHolder = cxx::ResourceHolder<&::FreeLibrary>;

public:
	// メッセージリソース用構造体
	struct SSelLangInfo {
		using Me = SSelLangInfo;

		std::filesystem::path	m_Path;				//!< メッセージリソースDLLのファイル名
		ResourceDllHolder		m_Module = nullptr;	//!< 読み込んだリソースのインスタンスハンドル
		WORD					m_LangId = 0;		//!< 言語ID
		std::wstring			m_LangName;			//!< 言語名

		SSelLangInfo() = default;

		explicit SSelLangInfo(const std::filesystem::path& path);

		SSelLangInfo(const Me&) = delete;
		Me& operator = (const Me&) = delete;

		SSelLangInfo(Me&&) noexcept = default;
		Me& operator = (Me&&) noexcept = default;

		~SSelLangInfo() noexcept;

		LPCWSTR	GetDllName() const noexcept { return m_Path.c_str(); }
		LPCWSTR	GetLangName() const noexcept { return m_LangName.c_str(); }

		bool	Load();
		void	Unload() noexcept;
	};

private:
	using LangInfoHolder = std::unique_ptr<SSelLangInfo>;

	using Me = CSelectLang;

public:
	static inline size_t gm_Selected = 0;
	static inline std::vector<LangInfoHolder> gm_Langs{};

	static HMODULE	InitializeLanguageEnvironment();

	static HMODULE	getLangRsrcInstance() noexcept;		// メッセージリソースDLLのインスタンスハンドルを返す
	static LPCWSTR	getDefaultLangString() noexcept;	// メッセージリソースDLL未読み込み時のデフォルト言語（"(Japanese)" or "(English(United States))"）
	static WORD		getDefaultLangId() noexcept;

	static std::span<LangInfoHolder> GetLangInfo() noexcept { return gm_Langs; }

	static const SSelLangInfo& GetLangInfo(size_t index) noexcept { return *gm_Langs[index].get(); }

	/*
	||  Constructors
	*/
	CSelectLang() noexcept = default;

	CSelectLang(const Me&) = delete;
	Me& operator = (const Me&) = delete;

	~CSelectLang() = default;

	/*
	||  Attributes & Operations
	*/
	static void		ChangeLang(const std::filesystem::path& dllName);

private:
	/*
	||  実装ヘルパ関数
	*/
	static void		ChangeLang(size_t newSelection);

public:
	static std::wstring_view LoadStringW(UINT id);
};

/*!
	@brief 文字列リソース読み込みクラス

	@date 2011.06.01 nasukoji	新規作成
*/
class CLoadString
{
private:
	// 文字列リソース読み込み用バッファクラス
	class CLoadStrBuffer
	{
	private:
		using Me = CLoadStrBuffer;

	public:
		CLoadStrBuffer() = default;

		CLoadStrBuffer(const Me&) = delete;		// コピー禁止とする
		Me& operator = (const Me&) = delete;	// 代入禁止とする

		~CLoadStrBuffer() = default;

		LPCWSTR	GetStringPtr() const noexcept { return m_String.c_str(); }	// 読み込んだ文字列のポインタを返す

		size_t	LoadStringW(UINT uid);								// 文字列リソースを読み込む（読み込み実行部）

	private:
		std::wstring_view	m_ResString;	//!< リソース文字列ビュー
		std::wstring		m_String;		//!< リソース文字列格納用バッファ
	};

	static inline std::array<CLoadStrBuffer, 4> gm_Buffers{};		//!< 文字列読み込みバッファの配列（CLoadString::LoadStringSt() が使用する）
	static inline size_t gm_LastUsedIndex = std::size(gm_Buffers);	//!< 最後に使用したバッファのインデックス（CLoadString::LoadStringSt() が使用する）

	using Me = CLoadString;

	CLoadStrBuffer m_Buffer;				//!< 文字列読み込みバッファ（CLoadString::LoadString() が使用する）

public:
	static LPCWSTR LoadStringSt(UINT uid);	//!< 静的バッファに文字列リソースを読み込む（各国語メッセージリソース対応）

	/*
	||  Constructors
	*/
	CLoadString() = default;

	CLoadString(const Me&) = delete;		//!< コピー禁止とする
	Me& operator = (const Me&) = delete;	//!< 代入禁止とする

	~CLoadString() = default;

	/*
	||  Attributes & Operations
	*/
	LPCWSTR	LoadStringW(UINT uid);			//!< 文字列リソースを読み込む（各国語メッセージリソース対応）
};

// 文字列ロード簡易化テンプレート
template<typename T> requires (std::is_integral_v<T> || std::is_convertible_v<T, int>)
inline LPCWSTR LS(T id) { return CLoadString::LoadStringSt(UINT(id)); }

#endif /* SAKURA_CSELECTLANG_657416B2_2B3D_455C_AC28_8B86244F5F83_H_ */
