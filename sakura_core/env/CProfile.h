/*!	@file
	@brief INIファイル入出力

	@author D.S.Koba
	@date 2003-10-21 D.S.Koba メンバ関数の名前と引数をそのままにしてメンバ変数，関数の中身を書き直し
	@date 2004-01-10 D.S.Koba 返値をBOOLからboolへ変更。IOProfileDataを型別の関数に分け，引数を減らす
	@date 2006-02-11 D.S.Koba 読み込み/書き出しを引数でなく，メンバで判別
	@date 2006-02-12 D.S.Koba IOProfileDataの中身の読み込みと書き出しを関数に分ける
*/
/*
	Copyright (C) 2003-2006, D.S.Koba
	Copyright (C) 2018-2025, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CPROFILE_1871E9A6_3FD3_45B5_A67D_6CC42F60363E_H_
#define SAKURA_CPROFILE_1871E9A6_3FD3_45B5_A67D_6CC42F60363E_H_
#pragma once

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief INIファイル入出力
*/
class CProfile
{
private:
	struct SectionType
	{
		using EntriesType = std::map< std::wstring, std::wstring >;

		explicit SectionType(
			std::wstring_view name
		) : m_Name(name)
		{
		}

		std::wstring	m_Name;
		EntriesType		m_Entries{};
	};

public:
	CProfile() = default;
	virtual ~CProfile() = default;

	bool IsReadingMode() const noexcept { return m_ReadingMode; }
	bool IsWritingMode() const noexcept { return !IsReadingMode(); }
	void SetReadingMode() noexcept { m_ReadingMode = true; }
	void SetWritingMode() noexcept { m_ReadingMode = false; }

	bool ReadProfileRes( const WCHAR*, const WCHAR*, std::vector<std::wstring>* = nullptr );				// 200/5/19 Uchi

	bool ReadProfile(const std::optional<std::filesystem::path>& optProfilePath = std::nullopt) noexcept;
	bool WriteProfile(const std::optional<std::filesystem::path>& optProfilePath = std::nullopt, const std::optional<std::wstring>& optComment = std::nullopt);

	bool GetProfileData(const std::wstring& sectionName, const std::wstring& entryKey, std::wstring& strEntryValue) const;
	void SetProfileData(const std::wstring& sectionName, const std::wstring& entryKey, std::wstring_view entryValue);

	void DUMP( void );

private:
	void _ReadOneline(const std::wstring& line);
	bool _WriteFile(const std::filesystem::path& path, std::span<const std::wstring> lines);

	// メンバ変数
	std::filesystem::path		m_ProfilePath;			//!< 最後に読み書きしたファイル名
	bool						m_ReadingMode = true;	//!< モード(true=読み込み/false=書き出し)
	std::vector<SectionType>	m_ProfileData{};
};

#endif /* SAKURA_CPROFILE_1871E9A6_3FD3_45B5_A67D_6CC42F60363E_H_ */
