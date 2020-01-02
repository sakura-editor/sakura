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

#include "util/StaticType.h"
#include "CProfile.h"

/*!
 * 独自バッファ参照型
 *
 * 固定長の文字配列を標準stringのように扱うためのクラス
 */
class StringBufferW {
	wchar_t*	m_pData;			//!< 文字列バッファを指すポインタ
	uint32_t	m_cchDataLength;	//!< 有効文字列長
	uint32_t	m_cchDataCount;		//!< 文字列バッファのサイズ

public:
	/*!
	 * コンストラクタ
	 *
	 * StringBufferWのインスタンスを構築する
	 *
	 * @param [in] pszData 文字列バッファを指すポインタ
	 * @param [in] cchDataCount 文字列バッファの確保サイズ
	 */
	StringBufferW( wchar_t* pszData, size_t cchDataCount ) noexcept
		: m_pData( pszData )
		, m_cchDataLength( 0 )
		, m_cchDataCount( cchDataCount )
	{
		// 文字列ポインタとサイズが有効な場合、有効文字列長を求める
		if( m_pData != NULL && m_cchDataCount > 0 ) {
			m_cchDataLength = ::wcsnlen( m_pData, m_cchDataCount );
			// NUL終端がなかったら、強制的にNUL終端する
			if( m_cchDataLength == m_cchDataCount ){
				m_pData[--m_cchDataLength] = L'\0';
			}
		}
	}

	/*!
	 * テンプレートコンストラクタ
	 *
	 * StringBufferWのインスタンスを構築する
	 *
	 * @param [in] szData 固定長文字配列への参照
	 */
	template <uint32_t cchDataCount>
	StringBufferW( wchar_t( &szData )[cchDataCount] ) noexcept
		: StringBufferW( szData, cchDataCount )
	{
	}

	// このクラスはコピー禁止
	StringBufferW( const StringBufferW& ) = delete;
	StringBufferW& operator = ( const StringBufferW& ) = delete;

	/*!
	 * ムーブコンストラクタ
	 *
	 * 空のStringBufferWインスタンスを構築し、
	 * 引数で指定されたインスタンスとデータを入れ替える
	 */
	StringBufferW( StringBufferW&& other ) noexcept
		: StringBufferW( NULL, 0 )
	{
		*this = std::forward<StringBufferW>( other );
	}

	/*!
	 * ムーブ代入演算子
	 *
	 * 引数で指定されたインスタンスとデータを入れ替える
	 */
	StringBufferW& operator = ( StringBufferW&& rhs ) noexcept
	{
		std::swap( m_pData, rhs.m_pData );
		std::swap( m_cchDataLength, rhs.m_cchDataLength );
		std::swap( m_cchDataCount, rhs.m_cchDataCount );
		return *this;
	}

	const wchar_t* c_str() const noexcept { return m_pData; }		//!< 文字列ポインタ(C-Style)
	size_t length() const noexcept { return m_cchDataLength; }		//!< 有効文字列長
	size_t capacity() const noexcept { return m_cchDataCount; }		//!< 文字列バッファのサイズ

	/*!
	 * バッファの内容を指定した文字列で置き替える
	 *
	 * @param [in] pSrc 文字列(NULL指定不可)
	 */
	void assign( const wchar_t* pSrc )
	{
		::wcsncpy_s( m_pData, capacity(), pSrc, _TRUNCATE );
		m_cchDataLength = ::wcsnlen( c_str(), capacity() );
	}

	StringBufferW& operator = ( const wchar_t* rhs ) { assign( rhs ); return *this; }
};

/*!
 * 各種データ変換付きCProfile
 *
 * @date 2007/09/24 kobake データ変換部を子クラスに分離
 */
class CDataProfile : public CProfile{
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       データ変換部                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*!
	 * 文字列(標準string)から設定値を読み取る
	 *
	 * テンプレート定義は、32bit以下の符号付き整数型とenum型に対応している。
	 * それ以外の型で利用したい場合は、特殊化定義を書いて使う。
	 * 既存コードにあった32bit以下の符号なし整数型, bool, CLayoutInt, CLogicInt, wchar_t は特殊化済み。
	 */
	template<typename NumType, typename = std::enable_if_t<sizeof(NumType) <= sizeof(int32_t)>>
	static bool TryParse( const std::wstring& profile, NumType& value ) noexcept
	{
		if( profile.empty() ) return false;
		bool ret = false;
		const wchar_t* pStart = profile.data();
		wchar_t* pEnd = NULL;
		auto parsed = ::wcstol( pStart, &pEnd, 10 );
		if( pStart != pEnd ){
			value = static_cast<NumType>(parsed);
			ret = true;
		}
		return ret;
	}

	/*!
	 * 設定値を文字列(標準string)に変換する
	 *
	 * テンプレート定義は、組込の整数型とenum型に対応している。
	 * 整数以外の型で利用したい場合は、特殊化定義を書いて使う。
	 * 既存コードにあった型 bool, CLayoutInt, CLogicInt, wchar_t は特殊化済み。
	 */
	template<typename NumType>
	static std::wstring ConvertToString( const NumType value ) noexcept
	{
		if constexpr (std::is_enum_v<NumType>) {
			return std::to_wstring( static_cast<int32_t>(value) );
		}else{
			return std::to_wstring( value );
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         入出力部                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*!
	 * 設定値の入出力テンプレート
	 *
	 * 標準stringを介して設定値の入出力を行う。
	 */
	template <class T> //T=={整数型, enum型, bool, wchar_t}
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
			if( GetProfileDataImp( pszSectionName, pszEntryKey, buf ) ){
				//Tに変換
				ret = TryParse( buf, tEntryValue );
			}
		}else{
			//文字列に変換
			buf = ConvertToString( tEntryValue );
			//文字列書き込み
			ret = SetProfileDataImp( pszSectionName, pszEntryKey, buf );
		}
		return ret;
	}

	/*!
	 * 独自バッファ参照型の入出力(標準stringを介して入出力)
	 *
	 * 引数型を右辺値参照(StringBufferW&&)としたいために通常入出力と分離。
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めたが長すぎて切り捨てられた
	 * @retval false 設定値が存在しなかったため読込できなかった
	 */
	bool IOProfileData(
		const WCHAR*			pszSectionName,	//!< [in] セクション名
		const WCHAR*			pszEntryKey,	//!< [in] エントリ名
		StringBufferW&&			refEntryValue	//!< [in,out] エントリ値
	) noexcept
	{
		// 標準stringに変換して入出力する
		std::wstring buf;

		bool ret = false;
		if( IsReadingMode() ){
			// 文字列読み込み
			if( GetProfileDataImp( pszSectionName, pszEntryKey, buf ) ){
				//StringBufferWに変換
				refEntryValue = buf.c_str();
				ret = buf.length() < refEntryValue.capacity();
			}
		}else{
			// 文字列に変換
			buf = refEntryValue.c_str();
			// 文字列書き込み
			ret = SetProfileDataImp( pszSectionName, pszEntryKey, buf );
		}
		return ret;
	}

	/*!
	 * 独自定義文字配列拡張型(StaticString)の入出力(標準stringを介して入出力)
	 *
	 * 型引数が合わないために通常入出力と分離。
	 * @retval true	設定値を正しく読み書きできた
	 * @retval false 設定値を読み込めたが長すぎて切り捨てられた
	 * @retval false 設定値が存在しなかったため読込できなかった
	 */
	template <int N>
	bool IOProfileData(
		const WCHAR*			pszSectionName,	//!< [in] セクション名
		const WCHAR*			pszEntryKey,	//!< [in] エントリ名
		StaticString<WCHAR, N>&	szEntryValue	//!< [in,out] エントリ値
	) noexcept
	{
		// 標準stringに変換して入出力する
		std::wstring buf;

		bool ret = false;
		if( IsReadingMode() ){
			//文字列読み込み
			if( IOProfileData( pszSectionName, pszEntryKey, buf ) ){
				//StaticString<WCHAR, N>に変換
				szEntryValue = buf.c_str();
				ret = buf.length() < _countof2(szEntryValue);
			}
		}else{
			//文字列に変換
			buf = szEntryValue.GetBufferPointer();
			//文字列書き込み
			ret = IOProfileData( pszSectionName, pszEntryKey, buf );
		}
		return ret;
	}
};

/*!
 * 文字列型(標準string)の入出力(無変換)
 */
template <> inline
bool CDataProfile::IOProfileData<std::wstring>(
	const WCHAR*			pszSectionName,	//!< [in] セクション名
	const WCHAR*			pszEntryKey,	//!< [in] エントリ名
	std::wstring&			strEntryValue	//!< [in,out] エントリ値
) noexcept
{
	bool ret = false;
	if( IsReadingMode() ){
		//文字列読み込み
		ret = GetProfileDataImp( pszSectionName, pszEntryKey, strEntryValue );
	}else{
		//文字列書き込み
		ret = SetProfileDataImp( pszSectionName, pszEntryKey, strEntryValue );
	}
	return ret;
}

/*!
 * 文字列(標準string)から設定値を読み取る
 *
 * 既存コードにあった32bit以下の符号なし整数型(WORD,DWORD等)を読み取るための特殊化。
 */
template<> inline
bool CDataProfile::TryParse<uint32_t>( const std::wstring& profile, uint32_t& value ) noexcept
{
	if (profile.empty()) return false;
	bool ret = false;
	const wchar_t* pStart = profile.data();
	wchar_t* pEnd = NULL;
	auto parsed = ::wcstoul( pStart, &pEnd, 10 );
	if( pStart != pEnd ){
		value = parsed;
		ret = true;
	}
	return ret;
}

#ifdef USE_STRICT_INT

//! 文字列(標準string)から設定値を読み取る
template<> inline
bool CDataProfile::TryParse<CLayoutInt>( const std::wstring& profile, CLayoutInt& value ) noexcept
{
	bool ret = false;
	int32_t nValue = -1;
	if( TryParse( profile, nValue ) ){
		value = CLayoutInt( nValue );
		ret = true;
	}
	return ret;
}

//! 設定値を文字列(標準string)に変換する
template<> inline
std::wstring CDataProfile::ConvertToString<CLayoutInt>( const CLayoutInt value ) noexcept
{
	return std::to_wstring( value.GetValue() );
}

//! 文字列(標準string)から設定値を読み取る
template<> inline
bool CDataProfile::TryParse<CLogicInt>( const std::wstring& profile, CLogicInt& value ) noexcept
{
	bool ret = false;
	int nValue = -1;
	if (TryParse( profile, nValue )) {
		value = CLogicInt( nValue );
		ret = true;
	}
	return ret;
}

//! 設定値を文字列(標準string)に変換する
template<> inline
std::wstring CDataProfile::ConvertToString<CLogicInt>( const CLogicInt value ) noexcept
{
	return std::to_wstring( value.GetValue() );
}
#endif //ifdef USE_STRICT_INT


//! 文字列(標準string)から設定値を読み取る
template<> inline
bool CDataProfile::TryParse<bool>( const std::wstring& profile, bool& value ) noexcept
{
	bool ret = false;
	int nValue = -1;
	if( TryParse( profile, nValue ) ) {
		value = (nValue != 0);
		ret = true;
	}
	return ret;
}

//! 設定値を文字列(標準string)に変換する
template<> inline
std::wstring CDataProfile::ConvertToString<bool>( const bool value ) noexcept
{
	return ConvertToString( value ? 1 : 0 );
}

/*!
 * 文字列(標準string)から設定値を読み取る
 *
 * 行番号エリアとテキストエリアを区切るのに使う文字の設定で使われている。
 *
 * @sa STypeConfig::m_cLineTermChar
 */
template<> inline
bool CDataProfile::TryParse<WCHAR>( const std::wstring& profile, WCHAR& value ) noexcept
{
	bool ret = false;
	if( profile.length() > 0 ){
		value = profile[0];
		ret = true;
	}
	return ret;
}

//! 設定値を文字列(標準string)に変換する
template<> inline
std::wstring CDataProfile::ConvertToString<WCHAR>( const WCHAR value ) noexcept
{
	return std::wstring( 1, value );
}

/*[EOF]*/
