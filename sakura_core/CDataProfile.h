/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CDATAPROFILE_401640FD_5B27_454A_B0DE_098E1C4FAEAD_H_
#define SAKURA_CDATAPROFILE_401640FD_5B27_454A_B0DE_098E1C4FAEAD_H_
#pragma once

#include "CProfile.h"

#include "basis/SakuraBasis.h"
#include "util/StaticType.h"

/*!
 * バッファ参照型
 *
 * 読み書き可能なWCHARバッファとサイズを指定して構築する
 */
using StringBufferW = basis::TCharBuffer<WCHAR>;

/*!
 * プロファイル用データ変換
 */
namespace profile_data {

	/*!
	 * 指定した型の変換に対応しているかどうかを返すインライン関数
	 */
	template<class T>
	constexpr bool is_supported_v = std::is_integral_v<T> || std::is_enum_v<T>;

	/*!
	 * 文字列を設定値に変換する
	 *
	 * テンプレート定義は、32bit以下の整数型とenum型に対応している。
	 * それ以外の型で利用したい場合は、特殊化定義を書いて使う。
	 * 既存コードにあった bool, WCHAR, KEYCODE は特殊化済み。
	 */
	template<class NumType>
	[[nodiscard]] bool TryParse(std::wstring_view profile, NumType& value) noexcept
	{
		if (profile.empty()) {
			return false;
		}

		const WCHAR* pStart = profile.data();
		WCHAR* pEnd = nullptr;
		if constexpr (std::is_signed_v<NumType>) {
			if (const auto parsedNum = wcstoll(pStart, &pEnd, 10); pStart != pEnd && parsedNum <= std::numeric_limits<NumType>::max() && std::numeric_limits<NumType>::min() <= parsedNum) {
				value = static_cast<NumType>(parsedNum);
				return true;
			}
		}
		else if constexpr (std::is_unsigned_v<NumType>) {
			if (const auto parsedNum = wcstoull(pStart, &pEnd, 10); pStart != pEnd && parsedNum <= std::numeric_limits<NumType>::max()) {
				value = static_cast<NumType>(parsedNum);
				return true;
			}
		}
		else if constexpr (std::is_enum_v<NumType>) {
			if (int32_t parsedNum; TryParse(profile, parsedNum)) {
				value = static_cast<NumType>(parsedNum);
				return true;
			}
		}

		return false;
	}

	/*!
	 * 設定値を文字列に変換する
	 *
	 * テンプレート定義は、組込の整数型に対応している。
	 * 整数以外の型で利用したい場合は、特殊化定義を書いて使う。
	 * 既存コードにあった型 bool, WCHAR, KEYCODE は特殊化済み。
	 */
	template<class NumType>
	[[nodiscard]] std::wstring ToString(NumType value)
	{
		if constexpr (std::is_enum_v<NumType>) {
			const auto nValue = static_cast<int32_t>(value);
			return ToString(nValue);
		}
		else if constexpr (std::is_integral_v<NumType>) {
			return std::to_wstring(value);
		}
	}

	//! 文字列を設定値に変換する
	template<>
	[[nodiscard]] inline bool TryParse<bool>(std::wstring_view profile, bool& value) noexcept
	{
		// int型を介して値を読み取る
		if (int nValue = 0; TryParse(profile, nValue)) {
			value = (nValue != 0);
			return true;
		}
		return false;
	}

	//! 設定値を文字列に変換する
	template<>
	[[nodiscard]] inline std::wstring ToString<bool>(bool value)
	{
		// int型を介して文字列化する
		return ToString(value ? 1 : 0);
	}

	/*!
	 * 文字列を設定値に変換する
	 *
	 * 行番号エリアとテキストエリアを区切るのに使う文字の設定。
	 *
	 * @sa STypeConfig::m_cLineTermChar
	 */
	template<>
	[[nodiscard]] inline bool TryParse<WCHAR>(std::wstring_view profile, WCHAR& value) noexcept
	{
		// \0は区切り文字「なし」を意味する
		if (const WCHAR ch = profile.empty() ? L'\0' : profile[0]; (ch & 0xd800) != 0xd800) {
			// 区切り文字は表示可能な単一文字なので、変換不要。
			value = ch;
			return true;
		}
		return false;
	}

	/*!
	 * 設定値を文字列に変換する
	 *
	 * 行番号エリアとテキストエリアを区切るのに使う文字の設定。
	 *
	 * @sa STypeConfig::m_cLineTermChar
	 */
	template<>
	[[nodiscard]] inline std::wstring ToString<WCHAR>(WCHAR value)
	{
		if (value != L'\0' && (value & 0xd800) != 0xd800) {
			// 指定された文字1字のみで構成される文字列を返す
			return std::wstring(1, value);
		}
		return std::wstring();
	}

	/*!
	 * 文字列を設定値に変換する
	 *
	 * 元々ACHAR型の変換メソッドになっていたものを再定義。
	 * カスタムメニューのニーモニック設定に使われている。
	 *
	 * ニーモニックとは、「ファイル(F)」の F のこと。(Fには下線が付く)
	 * キーボードショートカットとして機能することから、メニューショートカットとも言う。
	 *
	 * @sa CommonSetting_CustomMenu::m_nCustMenuItemKeyArr
	 */
	template<>
	[[nodiscard]] inline bool TryParse<KEYCODE>(std::wstring_view profile, KEYCODE& value) noexcept
	{
		// \0はニーモニック「なし」を意味する
		if (const WCHAR ch = profile.empty() ? L'\0' : profile[0]; ch < 0x80) {
			// ニーモニックはASCIIの印字可能文字なので、CRT関数を使った変換は不要。
			value = static_cast<KEYCODE>(ch);
			return true;
		}
		return false;
	}

	//! 設定値を文字列に変換する
	template<>
	[[nodiscard]] inline std::wstring ToString<KEYCODE>(KEYCODE value)
	{
		// WCHAR型を介して文字列化する
		static_assert(std::numeric_limits<KEYCODE>::max() < 0x80);
		const WCHAR ch = (value < 0) ? L'\0' : static_cast<std::make_unsigned_t<KEYCODE>>(value);
		return ToString(ch);
	}

}; // end of namespace profile_data

/*!
 * 各種データ変換付きCProfile
 *
 * @date 2007/09/24 kobake データ変換部を子クラスに分離
 */
class CDataProfile : public CProfile {
public:
	using CProfile::GetProfileData;
	using CProfile::SetProfileData;

	/*!
	 * Profileから読み込んだ文字列を設定値に変換して取得する
	 *
	 * @retval true 成功
	 * @retval false 失敗
	 */
	template<class T, std::enable_if_t<profile_data::is_supported_v<T>, int> = 0>
	[[nodiscard]] bool GetProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		T&						tEntryValue		//!< [out] エントリ値
	) const
	{
		if (std::wstring strEntryValue; GetProfileData(sectionName, entryKey, strEntryValue)) {
			return profile_data::TryParse(strEntryValue, tEntryValue);
		}
		return false;
	}

	/*!
	 * Profileから読み込んだ設定値を取得する
	 *
	 * @retval true 成功
	 * @retval false 失敗
	 */
	[[nodiscard]] bool GetProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		StringBufferW&			szEntryValue	//!< [out] エントリ値
	) const
	{
		if (std::wstring strEntryValue; GetProfileData(sectionName, entryKey, strEntryValue) && strEntryValue.length() < std::size(szEntryValue)) {
			szEntryValue = strEntryValue;
			return true;
		}
		return false;
	}

	/*!
	 * 設定値を文字列に変換してProfileへ書き込む
	 */
	template<class T, std::enable_if_t<profile_data::is_supported_v<T>, int> = 0>
	void SetProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		const T&				tEntryValue		//!< [in] エントリ値
	)
	{
		const auto strEntryValue = profile_data::ToString(tEntryValue);
		SetProfileData(sectionName, entryKey, strEntryValue);
	}

	/*!
	 * 設定値をProfileへ書き込む
	 */
	void SetProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		const StringBufferW&	szEntryValue	//!< [in] エントリ値
	)
	{
		SetProfileData(sectionName, entryKey, szEntryValue.c_str());
	}

	/*!
	 * 設定値の入出力テンプレート
	 *
	 * 設定値の入出力を行う。
	 *
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めなかった
	 */
	template <class T, std::enable_if_t<profile_data::is_supported_v<T> || std::is_same_v<T, StringBufferW> || std::is_same_v<T, std::wstring>, int> = 0>
	bool IOProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		T&						tEntryValue		//!< [in,out] エントリ値
	)
	{
		if( IsReadingMode() ){
			return GetProfileData(sectionName, entryKey, tEntryValue);
		}else{
			SetProfileData(sectionName, entryKey, tEntryValue);
			return true;
		}
	}

#ifdef USE_STRICT_INT

	/*!
	 * 設定値の入出力テンプレート
	 *
	 * 設定値の入出力を行う。
	 *
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めなかった
	 */
	template <int N,bool B0,bool B1,bool B2,bool B3>
	bool IOProfileData(
		std::wstring_view					sectionName,	//!< [in] セクション名
		std::wstring_view					entryKey,		//!< [in] エントリ名
		CStrictInteger<N, B0, B1, B2, B3>&	tEntryValue		//!< [in,out] エントリ値
	)
	{
		bool ret = true;
		if (int nValue = tEntryValue.GetValue(); IsReadingMode()) {
			// int型を介して値を読み取る
			ret = GetProfileData(sectionName, entryKey, nValue);
			if (ret) {
				tEntryValue.SetValue(nValue);
			}
		} else {
			// int型を介して文字列化して設定する
			SetProfileData(sectionName, entryKey, nValue);
		}
		return ret;
	}

#endif //ifdef USE_STRICT_INT

	/*!
	 * 独自定義バッファ参照型(StringBufferW)の入出力
	 *
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めなかった
	 */
	bool IOProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		StringBufferW			refEntryValue	//!< [in,out] エントリ値
	)
	{
		return IOProfileData<StringBufferW>(sectionName, entryKey, refEntryValue);
	}

	/*!
	 * 独自定義文字配列拡張型(StaticString)の入出力
	 *
	 * 型引数が合わないために通常入出力と分離。
	 *
	 */
	template <int N>
	bool IOProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		basis::SString<N>&		szEntryValue	//!< [in,out] エントリ値
	)
	{
		// バッファ参照型に変換して入出力する
		return IOProfileData(sectionName, entryKey, StringBufferW(szEntryValue, std::size(szEntryValue)));
	}

	/*!
	 * 独自定義文字配列拡張型(StaticString)の入出力
	 *
	 * 型引数が合わないために通常入出力と分離。
	 *
	 */
	template <int N>
	bool IOProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		StaticString<N>&		szEntryValue	//!< [in,out] エントリ値
	)
	{
		// バッファ参照型に変換して入出力する
		return IOProfileData(sectionName, entryKey, StringBufferW(szEntryValue, std::size(szEntryValue)));
	}
};

#endif /* SAKURA_CDATAPROFILE_401640FD_5B27_454A_B0DE_098E1C4FAEAD_H_ */
