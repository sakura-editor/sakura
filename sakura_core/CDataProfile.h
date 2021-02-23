/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CDATAPROFILE_401640FD_5B27_454A_B0DE_098E1C4FAEAD_H_
#define SAKURA_CDATAPROFILE_401640FD_5B27_454A_B0DE_098E1C4FAEAD_H_
#pragma once

#include "CProfile.h"

#include "basis/SakuraBasis.h"
#include "debug/Debug2.h"
#include "util/StaticType.h"

/*!
 * バッファ参照型
 *
 * 読み書き可能なWCHARバッファとサイズを指定して構築する
 */
class StringBufferW {
	WCHAR*		pszData_;
	size_t		cchDataSize_;

public:
	explicit StringBufferW(WCHAR* pData, size_t maxCount);

	template <size_t N>
	explicit StringBufferW(WCHAR (&buffer)[N])
		: StringBufferW(buffer, N) {}

	[[nodiscard]] const WCHAR* c_str() const noexcept { return pszData_; }
	[[nodiscard]] size_t capacity() const noexcept { return cchDataSize_; }

	StringBufferW& operator = (std::wstring_view rhs);
};

/*!
 * プロファイル用データ変換
 */
namespace profile_data {

	/*!
	 * 指定した型の変換に対応しているかどうかを返すインライン関数
	 */
	template<class T>
	constexpr bool is_supported_v = std::is_integral_v<T> || std::is_enum_v<T> || std::is_same_v<T, CLogicInt> || std::is_same_v<T, CLayoutInt> || std::is_same_v<T, StringBufferW>;

	/*!
	 * 文字列を設定値に変換する
	 *
	 * テンプレート定義は、32bit以下の整数型とenum型に対応している。
	 * それ以外の型で利用したい場合は、特殊化定義を書いて使う。
	 * 既存コードにあった bool, CLayoutInt, WCHAR, KEYCODE は特殊化済み。
	 * 独自型 CLogicInt は int に暗黙変換できるので特殊化不要。
	 */
	template<class NumType>
	[[nodiscard]] bool TryParse(std::wstring_view profile, NumType& value) noexcept
	{
		if (profile.length() > 0) {
			const WCHAR* pStart = profile.data();
			WCHAR* pEnd = nullptr;
			if constexpr (std::is_unsigned_v<NumType>) {
				if (const auto parsedNum = ::wcstoul(pStart, &pEnd, 10); pStart != pEnd) {
					value = static_cast<NumType>(parsedNum);
					return true;
				}
			}
			else {
				if (const auto parsedNum = ::wcstol(pStart, &pEnd, 10); pStart != pEnd) {
					value = static_cast<NumType>(parsedNum);
					return true;
				}
			}
		}
		return false;
	}

	/*!
	 * 設定値を文字列に変換する
	 *
	 * テンプレート定義は、組込の整数型に対応している。
	 * 整数以外の型で利用したい場合は、特殊化定義を書いて使う。
	 * 既存コードにあった型 bool, CLayoutInt, WCHAR, KEYCODE は特殊化済み。
	 * 独自型 CLogicInt は int に暗黙変換できるので特殊化不要。
	 */
	template<class NumType, std::enable_if_t<!std::is_enum_v<NumType>, std::nullptr_t> = nullptr>
	[[nodiscard]] std::wstring ToString(NumType value)
	{
		return std::to_wstring(value);
	}

	//! 設定値を文字列に変換する
	template<class EnumType, std::enable_if_t<std::is_enum_v<EnumType>, std::nullptr_t> = nullptr>
	[[nodiscard]] std::wstring ToString(EnumType value)
	{
		auto nValue = static_cast<int32_t>(value);
		return ToString(nValue);
	}

#ifdef USE_STRICT_INT

	//! 文字列を設定値に変換する
	template<>
	[[nodiscard]] inline bool TryParse<CLayoutInt>(std::wstring_view profile, CLayoutInt& value) noexcept
	{
		// int型を介して値を読み取る
		if (int nValue = 0; TryParse(profile, nValue)) {
			value = CLayoutInt(nValue);
			return true;
		}
		return false;
	}

	//! 設定値を文字列に変換する
	template<>
	[[nodiscard]] inline std::wstring ToString<CLayoutInt>(CLayoutInt value)
	{
		// int型を介して文字列化する
		return ToString((Int)value);
	}

#endif //ifdef USE_STRICT_INT

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
		const WCHAR ch = value < 0 || value >= 0x80 ? L'\0' : value;
		return ToString(ch);
	}

	/*!
	 * 文字列を設定値に変換する
	 *
	 * @sa StringBufferW
	 */
	template<>
	[[nodiscard]] inline bool TryParse<StringBufferW>(std::wstring_view profile, StringBufferW& value) noexcept
	{
		if (profile.length() < value.capacity()) {
			value = profile;
			return true;
		}
		return false;
	}

	//! 設定値を文字列に変換する
	[[nodiscard]] std::wstring ToString(const StringBufferW& value);

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
	template<class T, std::enable_if_t<profile_data::is_supported_v<T>, std::nullptr_t> = nullptr>
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
	 * 設定値を文字列に変換してProfileへ書き込む
	 */
	template<class T, std::enable_if_t<profile_data::is_supported_v<T>, std::nullptr_t> = nullptr>
	void SetProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		const T					tEntryValue		//!< [in] エントリ値
	)
	{
		const std::wstring strEntryValue = profile_data::ToString(tEntryValue);
		SetProfileData(sectionName, entryKey, strEntryValue.data());
	}

	/*!
	 * 設定値の入出力テンプレート
	 *
	 * 設定値の入出力を行う。
	 *
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めなかった
	 */
	template <class T> //T=={int, bool, WCHAR, KEYCODE, StringBufferW}
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

	/*!
	 * 独自定義文字配列拡張型(StaticString)の入出力
	 *
	 * 型引数が合わないために通常入出力と分離。
	 *
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めなかった
	 */
	template <int N>
	bool IOProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		StaticString<WCHAR, N>&	szEntryValue	//!< [in,out] エントリ値
	)
	{
		// バッファ参照型に変換して入出力する
		return IOProfileData(sectionName, entryKey, StringBufferW(szEntryValue.GetBufferPointer(), szEntryValue.GetBufferCount()));
	}

	/*!
	 * 独自定義バッファ参照型(StringBufferW)の入出力(右辺値参照バージョン)
	 *
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めなかった
	 */
	bool IOProfileData(
		std::wstring_view		sectionName,	//!< [in] セクション名
		std::wstring_view		entryKey,		//!< [in] エントリ名
		StringBufferW&&			refEntryValue	//!< [in,out] エントリ値
	);
};

#endif /* SAKURA_CDATAPROFILE_401640FD_5B27_454A_B0DE_098E1C4FAEAD_H_ */
