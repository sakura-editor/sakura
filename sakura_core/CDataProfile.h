/*! @file */
/*
	Copyright (C) 2008, kobake

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
#pragma once

#include "CProfile.h"
#include "basis/SakuraBasis.h"	//CLayoutInt
#include "basis/CStringBuf.h"
#include "util/StaticType.h"	//StaticString
#include "debug/Debug2.h" //assert_warning

/*!
 * プロファイル用データ変換
 */
namespace profile_data {

	/*!
	 * 文字列(標準string)から設定値を読み取る
	 *
	 * テンプレート定義は、32bit以下の符号付き整数型とenum型に対応している。
	 * それ以外の型で利用したい場合は、特殊化定義を書いて使う。
	 * 既存コードにあった32bit以下の符号なし整数型, bool, CLayoutInt, CLogicInt, wchar_t, KEYCODE は特殊化済み。
	 */
	template<typename NumType>
	bool TryParse( const std::wstring& profile, NumType& value ) noexcept
	{
		if( profile.empty() ) return false;

		const wchar_t* pStart = profile.data();
		wchar_t* pEnd = NULL;
		const auto parsedNum = ::wcstol( pStart, &pEnd, 10 );

		if( pStart != pEnd ){
			value = static_cast<NumType>(parsedNum);
			return true;
		}

		return false;
	}

	/*!
	 * 設定値を文字列(標準string)に変換する
	 *
	 * テンプレート定義は、組込の整数型とenum型に対応している。
	 * 整数以外の型で利用したい場合は、特殊化定義を書いて使う。
	 * 既存コードにあった型 bool, CLayoutInt, CLogicInt, wchar_t, char は特殊化済み。
	 */
	template<typename NumType>
	std::wstring ToString( const NumType& value ) noexcept
	{
		if constexpr( std::is_enum_v<NumType> ){
			return std::to_wstring( static_cast<int32_t>(value) );
		}else{
			return std::to_wstring( value );
		}
	}

	/*!
	 * 文字列(標準string)から設定値を読み取る
	 *
	 * 既存コードにあった32bit以下の符号なし整数型(WORD,DWORD等)を読み取るための特殊化。
	 */
	template<> inline
	bool TryParse<uint32_t>( const std::wstring& profile, uint32_t& value ) noexcept
	{
		if( profile.empty() ) return false;

		const wchar_t* pStart = profile.data();
		wchar_t* pEnd = NULL;
		const auto parsedNum = ::wcstoul( pStart, &pEnd, 10 );

		if( pStart != pEnd ){
			value = parsedNum;
			return true;
		}

		return false;
	}

#ifdef USE_STRICT_INT

	//! 文字列(標準string)から設定値を読み取る
	template<> inline
	bool TryParse<CLayoutInt>( const std::wstring& profile, CLayoutInt& value ) noexcept
	{
		// int型を介して値を読み取る
		int32_t nValue = -1;
		const bool parsed = TryParse( profile, nValue );
		if( parsed ){
			value = CLayoutInt( nValue );
		}
		return parsed;
	}

	//! 設定値を文字列(標準string)に変換する
	template<> inline
		std::wstring ToString<CLayoutInt>( const CLayoutInt& value ) noexcept
	{
		// int型を介して文字列化する
		return ToString( (Int)value );
	}

#endif //ifdef USE_STRICT_INT

	//! 文字列(標準string)から設定値を読み取る
	template<> inline
	bool TryParse<bool>( const std::wstring& profile, bool& value ) noexcept
	{
		// int型を介して値を読み取る
		int nValue = -1;
		const bool parsed = TryParse( profile, nValue );
		if( parsed ) {
			value = (nValue != 0);
		}
		return parsed;
	}

	//! 設定値を文字列(標準string)に変換する
	template<> inline
	std::wstring ToString<bool>( const bool& value ) noexcept
	{
		// int型を介して文字列化する
		return ToString( value ? 1 : 0 );
	}

	/*!
	 * 文字列(標準string)から設定値を読み取る
	 *
	 * 共有メモリ内の WCHAR value[260] を読み取るためのもの。
	 *
	 * @sa StringBuf<T>
	 */
	template<> inline
	bool TryParse<CStringBufW>( const std::wstring& profile, CStringBufW& value ) noexcept
	{
		if( profile.length() < value.capacity() ){
			value = profile.c_str();
			return true;
		}
		return false;
	}

	//! 設定値を文字列(標準string)に変換する
	template<> inline
	std::wstring ToString<CStringBufW>( const CStringBufW& value ) noexcept
	{
		if( value.IsValid() ) return std::wstring( value.c_str(), value.length() );
		return std::wstring();
	}

	/*!
	 * 文字列(標準string)から設定値を読み取る
	 *
	 * 行番号エリアとテキストエリアを区切るのに使う文字の設定で使われている。
	 *
	 * @sa STypeConfig::m_cLineTermChar
	 */
	template<> inline
	bool TryParse<WCHAR>( const std::wstring& profile, WCHAR& value ) noexcept
	{
		const WCHAR ch = profile[0];
		if( ch == '\0' ){
			// 区切り文字「なし」
			value = ch;
			return true;
		}

		// サロゲートチェック
		bool isSurrogateChar = IsUtf16SurrogHi( ch ) || IsUtf16SurrogLow( ch );
		assert_warning( !isSurrogateChar );

		// 区切り文字にサロゲート範囲の文字を指定することはできない
		if( !isSurrogateChar ){
			// 区切り文字は表示可能な単一文字なので、変換不要。
			value = ch;
			return true;
		}

		return false;
	}

	//! 設定値を文字列(標準string)に変換する
	template<> inline
	std::wstring ToString<WCHAR>( const WCHAR& value ) noexcept
	{
		// NUL文字が指定されたら空文字列を返す
		if( value == '\0' ) return std::wstring();

		// サロゲートチェック
		bool isSurrogateChar = IsUtf16SurrogHi( value ) || IsUtf16SurrogLow( value );
		assert_warning( !isSurrogateChar );

		// サロゲート範囲の文字が指定されたら空文字列を返す
		if( isSurrogateChar ) return std::wstring();

		// 指定された文字1字のみで構成される文字列を返す
		return std::wstring( 1, value );
	}

	/*!
	 * 文字列(標準string)から設定値を読み取る
	 *
	 * 元々ACHAR型の変換メソッドになっていたものを再定義。
	 * カスタムメニューのニーモニック設定に使われている。
	 *
	 * ニーモニックとは、「ファイル(F)」の F のこと。(Fには下線が付く)
	 * キーボードショートカットとして機能することから、メニューショートカットとも言う。
	 * 
	 * @sa CommonSetting_CustomMenu::m_nCustMenuItemKeyArr
	 */
	template<> inline
	bool TryParse<KEYCODE>( const std::wstring& profile, KEYCODE& value ) noexcept
	{
		if( profile.empty() ){
			// ニーモニック「なし」
			value = '\0';
			return true;
		}

		// ASCII7bit文字かチェック
		bool isAscii7bitChar = profile[0] < 0x80;
		assert_warning( isAscii7bitChar );

		if( isAscii7bitChar ){
			// ニーモニックはASCIIの印字可能文字なので、CRT関数を使った変換は不要。
			value = static_cast<KEYCODE>(profile[0]);
			return true;
		}

		return false;
	}

	//! 設定値を文字列(標準string)に変換する
	template<> inline
	std::wstring ToString<KEYCODE>( const KEYCODE& value ) noexcept
	{
		// WCHAR型を介して文字列化する

		// ASCII7bit文字かチェック
		bool isAscii7bitChar = value < 0x80;
		assert_warning( isAscii7bitChar );

		// ニーモニックはASCIIの印字可能文字なので、CRT関数を使った変換は不要。
		WCHAR ch = isAscii7bitChar ? static_cast<WCHAR>(value) : '\0';
		return ToString( ch );
	}

}; // end of namespace profile_data

/*!
 * 各種データ変換付きCProfile
 *
 * @date 2007/09/24 kobake データ変換部を子クラスに分離
 */
class CDataProfile : public CProfile{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         入出力部                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*!
	 * 設定値の入出力テンプレート
	 *
	 * 標準stringを介して設定値の入出力を行う。
	 */
	template <class T> //T=={整数型, enum型, bool, WCHAR, KEYCODE}
	bool IOProfileData(
		const WCHAR*			pszSectionName,	//!< [in] セクション名
		const WCHAR*			pszEntryKey,	//!< [in] エントリ名
		T&						tEntryValue		//!< [in,out] エントリ値
	) noexcept
	{
		// 標準stringに変換して入出力する
		std::wstring buf;

		bool ret = false;
		if( IsReadingMode() ){
			//文字列読み込み
			ret = GetProfileDataImp( pszSectionName, pszEntryKey, buf );
			if constexpr( !std::is_same_v<T,std::wstring> ){
				if( ret ){
					//Tに変換
					ret = profile_data::TryParse( buf, tEntryValue );
				}
			}
		}else{
			if constexpr( !std::is_same_v<T,std::wstring> ){
				//文字列に変換
				buf = profile_data::ToString( tEntryValue );
			}
			//文字列書き込み
			ret = SetProfileDataImp( pszSectionName, pszEntryKey, buf );
		}
		return ret;
	}

	/*!
	 * 引数が右辺値参照(T&&)となる型のためのグルーコード
	 *
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めなかった
	 */
	template <class T>
	bool IOProfileData(
		const WCHAR*			pszSectionName,	//!< [in] セクション名
		const WCHAR*			pszEntryKey,	//!< [in] エントリ名
		T&&						refEntryValue	//!< [in,out] エントリ値
	) noexcept
	{
		// 右辺値参照引数(=左辺値)を使って入出力テンプレートを呼び出す
		return IOProfileData( pszSectionName, pszEntryKey, refEntryValue );
	}

	/*!
	 * 独自定義文字配列拡張型(StaticString)の入出力(標準stringを介して入出力)
	 *
	 * 型引数が合わないために通常入出力と分離。
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めなかった
	 */
	template <int N>
	bool IOProfileData(
		const WCHAR*			pszSectionName,	//!< [in] セクション名
		const WCHAR*			pszEntryKey,	//!< [in] エントリ名
		StaticString<WCHAR, N>&	szEntryValue	//!< [in,out] エントリ値
	) noexcept
	{
		// バッファ参照型に変換して入出力する
		return IOProfileData( pszSectionName, pszEntryKey, CStringBufW( szEntryValue.GetBufferPointer(), szEntryValue.GetBufferCount() ) );
	}

	//2007.08.14 kobake 追加
	//! intを介して任意型の入出力を行う
	template <class T>
	bool IOProfileData_WrapInt( const WCHAR* pszSectionName, const WCHAR* pszEntryKey, T& nEntryValue)
	{
		int n=nEntryValue;
		bool ret=this->IOProfileData( pszSectionName, pszEntryKey, n );
		nEntryValue=(T)n;
		return ret;
	}
};
